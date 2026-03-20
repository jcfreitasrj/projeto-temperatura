from flask import Flask, jsonify, request
from flask_cors import CORS
from serial_reader import SerialReader
import os
import threading

app = Flask(__name__)
CORS(app, origins=os.getenv('CORS_ORIGINS', 'http://localhost:3000').split(','))

# Inicializa a leitura serial
serial_reader = SerialReader()
serial_reader.start()

# (Opcional) Conexão com MongoDB
# from pymongo import MongoClient
# mongo_uri = os.getenv('MONGO_URI', 'mongodb://mongo:27017/')
# client = MongoClient(mongo_uri)
# db = client.temperatura_db
# colecao = db.leituras

@app.route('/temperatura', methods=['GET'])
def get_temperatura():
    """Retorna a última leitura do Arduino"""
    data = serial_reader.get_latest()
    return jsonify(data), 200

@app.route('/controle', methods=['POST'])
def controle():
    """Recebe comando do frontend e envia para o Arduino"""
    dados = request.get_json()
    if not dados or 'comando' not in dados:
        return jsonify({'erro': 'Comando não fornecido'}), 400
    
    comando = dados['comando']
    if comando in ['LIGAR', 'DESLIGAR']:
        sucesso = serial_reader.send_command(comando)
        if sucesso:
            return jsonify({'status': 'comando enviado', 'comando': comando}), 200
        else:
            return jsonify({'erro': 'Falha ao enviar comando (serial indisponível)'}), 503
    return jsonify({'erro': 'Comando inválido'}), 400

@app.route('/historico', methods=['GET'])
def get_historico():
    """Retorna histórico do MongoDB (se implementado)"""
    # Exemplo: leituras = list(colecao.find().sort('_id', -1).limit(100))
    # for leitura in leituras: leitura['_id'] = str(leitura['_id'])
    # return jsonify(leituras), 200
    return jsonify({'mensagem': 'Histórico não implementado'}), 501

@app.route('/health', methods=['GET'])
def health():
    return jsonify({'status': 'ok'}), 200

if __name__ == '__main__':
    try:
        app.run(host='0.0.0.0', port=int(os.getenv('PORT', 5000)), debug=os.getenv('FLASK_DEBUG', 'False') == 'True')
    finally:
        serial_reader.stop()
