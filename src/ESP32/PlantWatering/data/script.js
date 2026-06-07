
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);

let powerGraph;
let powerData = [];

let moistureGraph;
let moistureData = [];

let LEDState = true;
let LEDStateButton;

function initWebSocket(){
    console.log("Trying to open a WebSocket connection...");
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event){
    console.log("Connection Opened");
}

function onClose(event) {
    console.log('Connection Closed');
    setTimeout(initWebSocket, 2000);
}

let firstMoistureReceived = false;
function onMessage(event){
    const raw = event.data.trim(); // remove whitespace/newlines
    if (!raw) return;

    try {
        const obj = JSON.parse(raw);
        console.log(JSON.stringify(obj));

        if (obj.MoistureLevel !== undefined) {
            let moistureLevel = Number(obj.MoistureLevel);

            if (!isNaN(moistureLevel)) {
                firstMoistureReceived = true;
            }

            if (firstMoistureReceived) {
                let flippedMoisture = 1 - moistureLevel;
                flippedMoisture = Math.min(Math.max(flippedMoisture, 0), 1);
                updateMoistureGraph(flippedMoisture);
            }

        } 
        else if (obj.GrowLEDPower !== undefined) {
            if (typeof obj.GrowLEDPower === "number") {
                updatePowerGraph(obj.GrowLEDPower);
            } else if (Array.isArray(obj.GrowLEDPower)) {
                obj.GrowLEDPower.forEach(v => updatePowerGraph(v));
            }

        } 
        else if (obj.WateringLogs) {
            const logs = obj.WateringLogs;
            const ul = document.getElementById("WateringLogsTimes");

            Object.keys(logs)
                .sort((a,b) => parseInt(b.replace("Log","")) - parseInt(a.replace("Log","")))
                .forEach(key => {
                    const log = logs[key];
                    // Skip the counter property if it exists
                    if (key === "NewLogIndex") return;

                    const li = document.createElement("li");
                    li.textContent = `${log.Date} at ${log.Time} — ${log.Amount} oz (${log.IsManual ? "Manual" : "Automatic"})`;
                    ul.prepend(li);  // prepend so newest log is at the top
                });
        } 
        else if (obj.MoisturePercent){
            const val = obj.MoisturePercent;
            document.getElementById("MoistureLevelValue").textContent = `Estimated Moisture: ${val}%`;
            drawMoistureGauge(val);
        }
        else {
            console.log("Unknown JSON object received:", obj);
        }
    } catch (err) {
        console.error("Failed to parse JSON:", raw, err);
    }
}

function onLoad(event){
    initWebSocket();
    initPowerGraph();
    initMoistureGraph();

    LEDStateButton = document.getElementById("ToggleLEDButton");
    LEDStateButton.style.backgroundColor = "#00891bff";
}

// Set and Display Time
setInterval(() => {
    const now = new Date();
    const timeString = now.toLocaleTimeString('en-US', {
        hour: 'numeric',
        minute: 'numeric',
        hour12: true
    });
    document.getElementById("TimeValue").textContent = timeString;
}, 1000);

function drawMoistureGauge(value) {
    const canvas = document.getElementById("MoistureLevelCanvas");
    if (!canvas) return;

    const ctx = canvas.getContext("2d");
    const width = canvas.width;
    const height = canvas.height;

    ctx.clearRect(0, 0, width, height);

    // Create horizontal gradient: red -> yellow -> green
    const gradient = ctx.createLinearGradient(0, 0, width, 0);
    gradient.addColorStop(0, "red");
    gradient.addColorStop(0.5, "yellow");
    gradient.addColorStop(0.75, "green");
    gradient.addColorStop(1, "blue");

    // Draw the background gradient
    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, width, height);

    // Draw marker line at current value
    const markerX = (value / 100) * width;
    ctx.beginPath();
    ctx.moveTo(markerX, 0);
    ctx.lineTo(markerX, height);
    ctx.strokeStyle = "black";
    ctx.lineWidth = 2;
    ctx.stroke();
}

function initPowerGraph(){
    const ctx = document.getElementById("PowerCanvas").getContext("2d");

    powerGraph = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: "Power [mW]",
                data: [],
                borderWidth: 2,
                tension: 0.3
            }]
        },
        options: {
            animation: false,
            responsive: false,
            scales: {
                x: { display: false },
                y: { beginAtZero: true }
            }
        }
    });
}

let powerHistoryMax = 50;
function updatePowerGraph(value){

    // Store Data
    powerData.push(value);
    if (powerData.length > powerHistoryMax) {
        powerData.shift();
    }

    // Push to Graph
    powerGraph.data.labels = powerData.map((_,i) => i);
    powerGraph.data.datasets[0].data = powerData;
    powerGraph.update();
}


function initMoistureGraph(){
    const ctx = document.getElementById("MoistureCanvas").getContext("2d");

    moistureGraph = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: "Moisture %",
                data: [],
                borderWidth: 2,
                tension: 0.3
            }]
        },
        options: {
            animation: false,
            responsive: false,
            scales: {
                x: { display: false },
                y: { beginAtZero: true }
            }
        }
    });
}

let moistureHistoryMax = 50;
function updateMoistureGraph(value){

    // Store Data
    moistureData.push(value);
    if (moistureData.length > moistureHistoryMax) {
        moistureData.shift();
    }

    // Push to Graph
    moistureGraph.data.labels = moistureData.map((_,i) => i);
    moistureGraph.data.datasets[0].data = moistureData;
    moistureGraph.update();
}

function sendManualWaterCommand(){

    const btn = document.getElementById("ManualWaterButton");
    btn.disabled = true; // Disable  button to prevent multiple presses
    websocket.send("PUMP_ON");

    // Off after 2 seconds
    setTimeout(() => {
        websocket.send("PUMP_OFF");
        btn.disabled = false;
    }, 2000);
}

function sendToggleGrowLEDCommand(){
    if (!LEDStateButton) return; // safety
    if (!LEDState) {
        LEDStateButton.style.backgroundColor = "#00891bff";
        if (websocket && websocket.readyState === WebSocket.OPEN) websocket.send("LED_FULL");
    } else {
        LEDStateButton.style.backgroundColor = "#df0404ff";
        if (websocket && websocket.readyState === WebSocket.OPEN) websocket.send("LED_DIM0");
    }
    LEDState = !LEDState;
}