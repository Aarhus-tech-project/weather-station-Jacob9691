namespace weatherAPI.Models
{
    public class WeatherData
    {
        public long Timestamp { get; set; }
        public double Temperature { get; set; }
        public double Pressure { get; set; }
        public double Altitude { get; set; }
        public double Humidity { get; set; }
    }
}
