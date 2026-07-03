//Seleção dos elementos da interface (DOM)
const redDisplay = document.getElementById("redCounter");
const greenDisplay = document.getElementById("greenCounter");
const blueDisplay = document.getElementById("blueCounter");
const timeDisplay = document.getElementById("timeCounter");
const totalDisplay = document.getElementById("totalCounter");

const connectBtn = document.getElementById("btnConect");
const ipInput = document.getElementById("ipInput");

let socket;

connectBtn.addEventListener('click', () => {
    const ip = ipInput.value;
    if (!ip) {
        alert("Por favor, digite o IP do ESP32 primeiro!");
        return;
    }

    //Cria a conexão WebSocket com a porta 81 do ESP32
    socket = new WebSocket(`ws://${ip}:81/`);

    socket.onopen = function(e) {
        console.log("Conectado ao ESP32 via Wi-Fi!");
        connectBtn.innerText = "Conectado!";
        connectBtn.style.backgroundColor = "lightgreen";
    };

    socket.onmessage = function(event) {
        //Quando os dados chegam pela rede, são processados imediatamente
        processarDados(event.data);
    };

    socket.onclose = function(event) {
        console.log("Conexão perdida.");
        connectBtn.innerText = "Conectar Wi-Fi";
        connectBtn.style.backgroundColor = "";
    };
});

//ESP32 enviará o formato de texto
function processarDados(data) {
    const regex = /Tempo:\s*(\d+)s;\s*Vermelho:\s*(\d+);\s*Azul:\s*(\d+);\s*Verde:\s*(\d+);\s*Total:\s*(\d+);/;
    const match = data.match(regex);

    if (match) {
        timeDisplay.innerText = match[1] + "s";
        redDisplay.innerText = match[2];
        blueDisplay.innerText = match[3];
        greenDisplay.innerText = match[4];
        totalDisplay.innerText = match[5];
    }
}

const btnOnOff = document.getElementById("btnOnOff");
let esteiraEstaLigada = false;

btnOnOff.addEventListener('click', () => {
    esteiraEstaLigada = !esteiraEstaLigada;

    if (esteiraEstaLigada) {
        btnOnOff.innerText = "Desligar Esteira";
        btnOnOff.style.backgroundColor = "lightcoral"; 
    } else {
        btnOnOff.innerText = "Ligar Esteira";
        btnOnOff.style.backgroundColor = "lightgreen"; 
    }
});