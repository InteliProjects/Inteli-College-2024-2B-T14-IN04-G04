from flask import Flask, jsonify, request
from flask_mqtt import Mqtt
import psycopg2
from datetime import datetime
import json

# Configuração do Flask
app = Flask(__name__)

# Configuração MQTT
app.config['MQTT_BROKER_URL'] = 'broker.hivemq.com'
app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_USERNAME'] = 'Apontados'
app.config['MQTT_PASSWORD'] = 'Apontados123'
app.config['MQTT_KEEPALIVE'] = 60
app.config['MQTT_TLS_ENABLED'] = False

mqtt = Mqtt(app)

# Configuração do Banco de Dados PostgreSQL
DB_HOST = "dpg-cspqlnjv2p9s738u0560-a.oregon-postgres.render.com"
DB_NAME = "instituto_apontar"
DB_USER = "instituto_apontar_user"
DB_PASSWORD = "NqD6qhYNIm0X9TJvLPlsBTlRUIKIKClR"

def get_db_connection():
    """Estabelece uma conexão com o banco de dados PostgreSQL."""
    try:
        conn = psycopg2.connect(
            host=DB_HOST,
            dbname=DB_NAME,
            user=DB_USER,
            password=DB_PASSWORD,
            port="5432"
        )
        print("[INFO] Conexão ao banco de dados estabelecida com sucesso.")
        return conn
    except Exception as e:
        print("[ERRO] Falha ao conectar ao banco de dados:", e)
        raise
def create_tables():
    """Cria as tabelas cadastros e acessos no banco de dados PostgreSQL."""
    try:
        conn = get_db_connection()
        cursor = conn.cursor()
        # SQL para criar a tabela 'cadastros'
        create_cadastros_table = """
        CREATE TABLE IF NOT EXISTS cadastros (
            id SERIAL PRIMARY KEY,
            nome VARCHAR(255),
            hash_biometria TEXT NOT NULL,
            data_nascimento DATE,
            endereco TEXT,
            email VARCHAR(255),
            telefone VARCHAR(15),
            aluno BOOLEAN
        );
        """
        # SQL para criar a tabela 'acessos'
        create_acessos_table = """
        CREATE TABLE IF NOT EXISTS acessos (
            id SERIAL PRIMARY KEY,
            aluno_id INT NOT NULL,
            status VARCHAR(50),
            entrada TIMESTAMP NOT NULL,
            saida TIMESTAMP,
            tempo_permanencia INTERVAL
        );
        """
        cursor.execute(create_cadastros_table)
        cursor.execute(create_acessos_table)
        conn.commit()
        cursor.close()
        conn.close()
        print("[INFO] Tabelas 'cadastros' e 'acessos' criadas com sucesso.")
    except Exception as e:
        print(f"[ERRO] Falha ao criar tabelas: {e}")

@mqtt.on_connect()
def on_connect(client, userdata, flags, rc):
    print(f"[INFO]Conectado ao MQTT com código {rc}")
    client.subscribe("instituto/apontar/biometria")  # Se inscreve no tópico 'instituto/apontar/biometria'

# Callback para mensagens MQTT
@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    """Processa mensagens recebidas via MQTT."""
    try:
        payload = message.payload.decode()
        data = json.loads(payload)

        print(f"[INFO] Mensagem recebida via MQTT: {data}")
        print(f"[INFO] Tópico: {message.topic}")

        if message.topic == "instituto/apontar/biometria":
            if "id" in data and "hash_biometria" in data:
                aluno_id = data["id"]
                hash_biometria = data["hash_biometria"]
                print(f"[INFO] Processando ID: {aluno_id}, Hash: {hash_biometria}")

                # Salvar a biometria no banco de dados
                conn = get_db_connection()
                cursor = conn.cursor()
                cursor.execute(
                    """
                    INSERT INTO cadastros (id, hash_biometria)
                    VALUES (%s, %s)
                    ON CONFLICT (id) DO UPDATE SET hash_biometria = EXCLUDED.hash_biometria;
                    """,
                    (aluno_id, hash_biometria)
                )
                conn.commit()
                cursor.close()
                conn.close()
                print(f"[INFO] Biometria salva no banco para o ID: {aluno_id}")
            else:
                print("[ERRO] Dados inválidos no tópico 'instituto/apontar/biometria'.")
    except Exception as e:
        print(f"[ERRO] Falha ao processar mensagem MQTT: {e}")

# Rotas HTTP para gerenciamento de cadastros e acessos
@app.route('/cadastros', methods=['GET'])
def get_cadastros():
    """Retorna todos os cadastros de biometria."""
    try:
        conn = get_db_connection()
        cursor = conn.cursor()
        cursor.execute("SELECT * FROM cadastros")
        cadastros = cursor.fetchall()
        cursor.close()
        conn.close()
        return jsonify(cadastros), 200
    except Exception as e:
        print(f"[ERRO] Falha ao buscar cadastros: {e}")
        return jsonify({"status": "Erro", "message": "Falha ao buscar cadastros"}), 500
@app.route('/cadastro', methods=['POST'])
def add_cadastro():
    """Adiciona um novo cadastro manualmente."""
    try:
        data = request.json
        if not data.get("hash_biometria"):
            return jsonify({"status": "Erro", "message": "Dados obrigatórios ausentes"}), 400
        conn = get_db_connection()
        cursor = conn.cursor()
        cursor.execute(
            """
            INSERT INTO cadastros (nome, hash_biometria, data_nascimento, endereco, email, telefone, aluno)
            VALUES (%s, %s, %s, %s, %s, %s, %s)
            RETURNING id;
            """,
            (
                data.get("nome"),
                data["hash_biometria"],
                data.get("data_nascimento"),
                data.get("endereco"),
                data.get("email"),
                data.get("telefone"),
                data.get("aluno", True)
            )
        )
        cadastro_id = cursor.fetchone()[0]
        conn.commit()
        cursor.close()
        conn.close()
        return jsonify({"status": "Cadastro realizado com sucesso!", "id": cadastro_id}), 201
    except Exception as e:
        print(f"[ERRO] Falha ao adicionar cadastro: {e}")
        return jsonify({"status": "Erro", "message": f"Falha ao adicionar cadastro: {str(e)}"}), 500
# Inicialização da aplicação Flask
if __name__ == "__main__":
    print("[INFO] Inicializando aplicação Flask...")
    # Criar as tabelas antes de iniciar o Flask
    create_tables()
    # Inicializar MQTT e executar o Flask
    mqtt.init_app(app)
    app.run(host='0.0.0.0', port=5000, debug=True)