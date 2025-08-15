async function loadData() {
    try {
        const res = await fetch('/api/weather');
        const data = await res.json();

        if (data.length > 0) {
            const latest = data[0];

            document.getElementById('curTemp').textContent = latest.temperature.toFixed(2);
            document.getElementById('curHumidity').textContent = latest.humidity.toFixed(2);
            document.getElementById('curPressure').textContent = latest.pressure.toFixed(2);
            document.getElementById('curAltitude').textContent = latest.altitude.toFixed(2);
        }

        const labels = data.map(d => {
            const ts = d.timestamp.toString();
            const year = parseInt(ts.slice(0, 4));
            const month = parseInt(ts.slice(4, 6)) - 1; // 0-based
            const day = parseInt(ts.slice(6, 8));
            const hour = parseInt(ts.slice(8, 10));
            const minute = parseInt(ts.slice(10, 12));
            const second = parseInt(ts.slice(12, 14));
            const date = new Date(year, month, day, hour, minute, second);
            return date.toLocaleString();
        }).reverse();

        const datasets = [
            { id: 'tempChart', label: 'Temperature (°C)', data: data.map(d => d.temperature).reverse(), color: 'red', yLabel: 'Temperature (°C)' },
            { id: 'humidityChart', label: 'Humidity (%)', data: data.map(d => d.humidity).reverse(), color: 'blue', yLabel: 'Humidity (%)' },
            { id: 'pressureChart', label: 'Pressure (hPa)', data: data.map(d => d.pressure).reverse(), color: 'green', yLabel: 'Pressure (hPa)' }
        ];

        function createLineChart({ id, label, data, color, yLabel }) {
            new Chart(document.getElementById(id), {
                type: 'line',
                data: {
                    labels,
                    datasets: [{
                        label, data, borderColor: color, borderWidth: 2, fill: false, tension: 0.1
                    }]
                },
                options: {
                    responsive: true,
                    plugins: { legend: { display: true } },
                    scales: {
                        x: { title: { display: true, text: 'Time' } },
                        y: { title: { display: true, text: yLabel } }
                    }
                }
            });
        }

        datasets.forEach(createLineChart);
    } catch (err) {
        console.error('Error loading data', err);
    }
}

loadData();