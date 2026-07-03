""" Terminal do Rapberry Pi
sudo apt update
sudo apt install python3-pip
pip3 install websockets adafruit-circuitpython-tcs34725 RPLCD gpiozero """

import asyncio
import websockets
import board
import busio
import adafruit_tcs34725
from gpiozero import AngularServo
from RPLCD.i2c import CharLCD
import time

#Variáveis
red = 0
green = 0
blue = 0
tempo = 0
esteira_ligada = False
clientes_conectados = set()

#Hardware
#Inicializa o barramento I2C (Pinos SDA=3, SCL=5 físicos do Pi)
i2c = board.I2C()

#Configura o Sensor de Cor
tcs = adafruit_tcs34725.TCS34725(i2c)
tcs.integration_time = 24
tcs.gain = 16

#Configura o LCD I2C (Endereço 0x27)
lcd = CharLCD(i2c_expander='PCF8574', address=0x27, port=1, cols=16, rows=2)

#Configura os Servos Motores nos pinos GPIO (17, 27, 22)
servo_red = AngularServo(17, min_angle=-90, max_angle=90)
servo_green = AngularServo(27, min_angle=-90, max_angle=90)
servo_blue = AngularServo(22, min_angle=-90, max_angle=90)

#Coloca os servos na posição inicial de 0 graus
servo_red.angle = -90
servo_green.angle = -90
servo_blue.angle = -90

#Pino para ligar/desligar o motor principal da esteira (GPIO 23)
#motor_esteira = OutputDevice(23)

lcd.clear()
lcd.write_string('Esteira Pi\nIniciando...')
time.sleep(2)

#Funções de rede (Websocket)
async def enviar_dados():
    #Monta a string no formato que o script.js espera
    if clientes_conectados:
        total = red + green + blue
        payload = f"Tempo: {tempo}s; Vermelho: {red}; Azul: {blue}; Verde: {green}; Total: {total};\n"
        #Envia para todos os sites conectados
        await asyncio.gather(*(cliente.send(payload) for cliente in clientes_conectados))

async def gerenciar_conexao(websocket, path):
    #Recebe comandos do site (CMD:ON / CMD:OFF)
    global esteira_ligada
    clientes_conectados.add(websocket)
    print("Novo site conectado!")
    try:
        async for mensagem in websocket:
            if mensagem == "CMD:ON":
                esteira_ligada = True
                #motor_esteira.on() #Liga motor físico
                print("Comando Web: Esteira LIGADA")
            elif mensagem == "CMD:OFF":
                esteira_ligada = False
                #motor_esteira.off() #Desliga motor físico
                print("Comando Web: Esteira DESLIGADA")
    finally:
        clientes_conectados.remove(websocket)

#Funções mecânicas
async def mover_servo(servo, cor_nome):
    #Move o servo e atualiza contadores
    global red, green, blue
    print(f"Detectado: {cor_nome}")
    
    #Adiciona +1 na cor correta
    if cor_nome == "Vermelho": red += 1
    elif cor_nome == "Verde": green += 1
    elif cor_nome == "Azul": blue += 1

    await enviar_dados() #Atualiza o site instantaneamente
    
    #Movimento do servo
    servo.angle = 60
    await asyncio.sleep(1.25) #equivale ao dela_ = 1250
    servo.angle = -90

#Loop principal do hardware
async def loop_hardware():
    global tempo
    ultimo_tempo = time.time()

    while True:
        if esteira_ligada:
            #Atualiza o relógio a cada 1 segundo
            agora = time.time()
            if agora - ultimo_tempo >= 1.0:
                tempo += 1
                ultimo_tempo = agora
                await enviar_dados()
            
            #Leitura do Sensor de Cores
            r, g, b, c = tcs.color_raw
            soma_cores = r + g + b
            
            if soma_cores > 815:
                if (r > g) and (r > b):
                    await mover_servo(servo_red, "Vermelho")
                elif (g > r) and (g > b):
                    await mover_servo(servo_green, "Verde")
                elif (b > r) and (b > g):
                    await mover_servo(servo_blue, "Azul")
        
        #Atualiza o LCD físico
        lcd.cursor_pos = (0, 0)
        lcd.write_string(f"R:{red:<3} G:{green:<3}")
        lcd.cursor_pos = (1, 0)
        lcd.write_string(f"B:{blue:<3} T:{tempo:<3}s")
        
        await asyncio.sleep(0.05) #Pausa mínima para não sobrecarregar a CPU do Pi

#Inicialização geral
async def main():
    #Inicia o servidor WebSocket na porta 81
    servidor_ws = websockets.serve(gerenciar_conexao, "0.0.0.0", 81)
    
    print("Servidor rodando. Aguardando conexão Wi-Fi...")
    
    #Roda o servidor e o loop de hardware simultaneamente
    await asyncio.gather(
        servidor_ws,
        loop_hardware()
    )

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nPrograma encerrado.")
        lcd.clear()

""" Para rodar o código: Basta abrir o terminal do Pi e digitar "python3 esteira_pi.py".
Para descobrir o IP do seu Raspberry Pi e colocar no site, digite hostname -I no terminal. """