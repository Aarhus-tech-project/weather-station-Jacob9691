#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>
#include <mysql/mysql.h>
#include <cjson/cJSON.h>

// MQTT details
#define MQTT_HOST "localhost"
#define MQTT_PORT 1883
#define MQTT_TOPIC "weather/station1"

// Struct to hold shared DB connection
typedef struct {
    MYSQL *conn;
} AppContext;

// Function to handle incoming MQTT messages
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
    AppContext *ctx = (AppContext *)obj;
    MYSQL *conn = ctx->conn;

    const char *payload = (const char *)message->payload;

    cJSON *json = cJSON_Parse(payload);
    if (!json) {
        fprintf(stderr, "Failed to parse JSON: %s\n", payload);
        return;
    }

    cJSON *ts = cJSON_GetObjectItemCaseSensitive(json, "timestamp");
    cJSON *temp = cJSON_GetObjectItemCaseSensitive(json, "temperature");
    cJSON *press = cJSON_GetObjectItemCaseSensitive(json, "pressure");
    cJSON *alt = cJSON_GetObjectItemCaseSensitive(json, "altitude");
    cJSON *humid = cJSON_GetObjectItemCaseSensitive(json, "humidity");

    if (!cJSON_IsNumber(ts) || !cJSON_IsNumber(temp) || !cJSON_IsNumber(press) ||
        !cJSON_IsNumber(alt) || !cJSON_IsNumber(humid)) {
        fprintf(stderr, "Invalid data types in JSON: %s\n", payload);
        cJSON_Delete(json);
        return;
    }

    // Prepare the SQL INSERT
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[5];
    const char *query = "INSERT INTO weatherdata (timestamp, temperature, pressure, altitude, humidity) VALUES (?, ?, ?, ?, ?)";

    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        cJSON_Delete(json);
        return;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        cJSON_Delete(json);
        return;
    }

    memset(bind, 0, sizeof(bind));

    // Bind data
    long long timestamp = (long long)ts->valuedouble;
    float temperature = (float)temp->valuedouble;
    float pressure = (float)press->valuedouble;
    float altitude = (float)alt->valuedouble;
    float humidity = (float)humid->valuedouble;

    bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[0].buffer = &timestamp;

    bind[1].buffer_type = MYSQL_TYPE_FLOAT;
    bind[1].buffer = &temperature;

    bind[2].buffer_type = MYSQL_TYPE_FLOAT;
    bind[2].buffer = &pressure;

    bind[3].buffer_type = MYSQL_TYPE_FLOAT;
    bind[3].buffer = &altitude;

    bind[4].buffer_type = MYSQL_TYPE_FLOAT;
    bind[4].buffer = &humidity;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n", mysql_stmt_error(stmt));
    } else if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
    } else {
        printf("Data inserted: timestamp=%lld temp=%.2f press=%.2f alt=%.2f hum=%.2f\n",
               timestamp, temperature, pressure, altitude, humidity);
    }

    mysql_stmt_close(stmt);
    cJSON_Delete(json);
}

int main() {
    // Load MySQL credentials from environment variables
    const char *db_host = getenv("MYSQL_HOST");
    const char *db_user = getenv("MYSQL_USER");
    const char *db_pass = getenv("MYSQL_PASS");
    const char *db_name = getenv("MYSQL_DB");

    if (!db_host || !db_user || !db_pass || !db_name) {
        fprintf(stderr, "Missing MySQL environment variables.\n");
        return 1;
    }

    // Initialize MySQL
    mysql_library_init(0, NULL, NULL);
    MYSQL *conn = mysql_init(NULL);
    if (!conn) {
        fprintf(stderr, "mysql_init() failed\n");
        return 1;
    }

    if (!mysql_real_connect(conn, db_host, db_user, db_pass, db_name, 0, NULL, 0)) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(conn));
        return 1;
    }

    // Set up Mosquitto
    mosquitto_lib_init();
    AppContext ctx = { .conn = conn };
    struct mosquitto *mosq = mosquitto_new(NULL, true, &ctx);
    if (!mosq) {
        fprintf(stderr, "Failed to create Mosquitto client\n");
        return 1;
    }

    mosquitto_message_callback_set(mosq, on_message);

    if (mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 60) != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to connect to MQTT broker\n");
        return 1;
    }

    if (mosquitto_subscribe(mosq, NULL, MQTT_TOPIC, 0) != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Subscription failed\n");
        return 1;
    }

    printf("Connected. Listening for MQTT messages on topic '%s'...\n", MQTT_TOPIC);
    mosquitto_loop_forever(mosq, -1, 1);

    // Cleanup
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    mysql_close(conn);
    mysql_library_end();

    return 0;
}