import React, { useState, useEffect } from 'react';
import axios from 'axios';

// URL da API vinda de variável de ambiente ou fallback
const API_URL = process.env.REACT_APP_API_URL || 'http://localhost:5000';

function App() {
  const [temperatura, setTemperatura] = useState(null);
  const [umidade, setUmidade] = useState(null);
  const [timestamp, setTimestamp] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const [comandoEnviando, setComandoEnviando] = useState(false);
  const [mensagem, setMensagem] = useState('');

  // Busca os dados de temperatura no backend
  const fetchTemperatura = async () => {
    try {
      const response = await axios.get(`${API_URL}/temperatura`);
      setTemperatura(response.data.temperatura);
      setUmidade(response.data.umidade);
      setTimestamp(response.data.timestamp);
      setError(null);
    } catch (err) {
      setError('Erro ao buscar dados. Verifique a conexão com o backend.');
      console.error(err);
    } finally {
      setLoading(false);
    }
  };

  // Atualiza a cada 3 segundos
  useEffect(() => {
    fetchTemperatura();
    const interval = setInterval(fetchTemperatura, 3000);
    return () => clearInterval(interval);
  }, []);

  // Envia comando LIGAR/DESLIGAR para o backend
  const enviarComando = async (comando) => {
    setComandoEnviando(true);
    setMensagem('');
    try {
      await axios.post(`${API_URL}/controle`, { comando });
      setMensagem(`Comando ${comando} enviado com sucesso!`);
    } catch (err) {
      setMensagem('Erro ao enviar comando. Tente novamente.');
      console.error(err);
    } finally {
      setComandoEnviando(false);
      setTimeout(() => setMensagem(''), 3000); // limpa a mensagem após 3s
    }
  };

  // Exibe tela de carregamento
  if (loading) {
    return (
      <div style={styles.container}>
        <p>Carregando...</p>
      </div>
    );
  }

  // Exibe mensagem de erro (com estilos mesclados)
  if (error) {
    return (
      <div style={{ ...styles.container, color: 'red' }}>
        <p>{error}</p>
      </div>
    );
  }

  return (
    <div style={styles.container}>
      <h1>Controle de Temperatura</h1>

      <div style={styles.dataBox}>
        <p>
          <strong>Temperatura:</strong>{' '}
          {temperatura !== null ? `${temperatura} °C` : '---'}
        </p>
        <p>
          <strong>Umidade:</strong>{' '}
          {umidade !== null ? `${umidade} %` : '---'}
        </p>
        <p>
          <strong>Última atualização:</strong>{' '}
          {timestamp ? new Date(timestamp).toLocaleString() : '---'}
        </p>
      </div>

      <h2>Controle do Atuador</h2>

      {/* Feedback visual para os comandos */}
      {mensagem && (
        <div
          style={{
            ...styles.mensagem,
            color: mensagem.includes('Erro') ? 'red' : 'green',
          }}
        >
          {mensagem}
        </div>
      )}

      <div style={styles.buttonGroup}>
        <button
          onClick={() => enviarComando('LIGAR')}
          disabled={comandoEnviando}
          style={{
            ...styles.button,
            backgroundColor: '#4CAF50',
            opacity: comandoEnviando ? 0.5 : 1,
            cursor: comandoEnviando ? 'not-allowed' : 'pointer',
          }}
        >
          Ligar Ventilador
        </button>
        <button
          onClick={() => enviarComando('DESLIGAR')}
          disabled={comandoEnviando}
          style={{
            ...styles.button,
            backgroundColor: '#f44336',
            opacity: comandoEnviando ? 0.5 : 1,
            cursor: comandoEnviando ? 'not-allowed' : 'pointer',
          }}
        >
          Desligar Ventilador
        </button>
      </div>

      {/* Comentário explicativo: não há sensor de umidade, por isso pode aparecer "---" */}
    </div>
  );
}

// Estilos inline (sem pseudo-classes, que foram substituídas por lógica condicional)
const styles = {
  container: {
    padding: '20px',
    fontFamily: 'Arial, sans-serif',
    maxWidth: '600px',
    margin: '0 auto',
  },
  dataBox: {
    border: '1px solid #ccc',
    borderRadius: '8px',
    padding: '15px',
    margin: '20px 0',
    backgroundColor: '#f9f9f9',
  },
  buttonGroup: {
    display: 'flex',
    gap: '10px',
    justifyContent: 'center',
  },
  button: {
    padding: '10px 20px',
    fontSize: '16px',
    border: 'none',
    borderRadius: '4px',
    color: 'white',
    transition: 'opacity 0.3s',
  },
  mensagem: {
    marginTop: '10px',
    padding: '10px',
    border: '1px solid #ccc',
    borderRadius: '4px',
    backgroundColor: '#f0f0f0',
    textAlign: 'center',
  },
};

export default App;
