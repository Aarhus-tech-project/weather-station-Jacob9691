using Microsoft.AspNetCore.Mvc;
using MySql.Data.MySqlClient;
using weatherAPI.Models;

[ApiController]
[Route("api/[controller]")]
public class WeatherController : ControllerBase
{
    private readonly string? _connectionString;

    public WeatherController(IConfiguration config)
    {
        _connectionString = config.GetConnectionString("DefaultConnection");
    }

    [HttpGet]
    public IEnumerable<WeatherData> Get()
    {
        var list = new List<WeatherData>();
        using var conn = new MySqlConnection(_connectionString);
        conn.Open();
        using var cmd = new MySqlCommand("SELECT timestamp, temperature, pressure, altitude, humidity FROM weatherdata ORDER BY timestamp DESC LIMIT 100", conn);
        using var reader = cmd.ExecuteReader();
        while (reader.Read())
        {
            list.Add(new WeatherData
            {
                Timestamp = reader.GetInt64(0),
                Temperature = reader.GetDouble(1),
                Pressure = reader.GetDouble(2),
                Altitude = reader.GetDouble(3),
                Humidity = reader.GetDouble(4)
            });
        }
        return list;
    }
}
