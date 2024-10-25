import './App.css';
import Use_mem from './components/use_mem';
import Use_swap from './components/use_swap';
import Fault_pages from './components/fault_pages';
import Use_pages from './components/use_pages';
import Use_mem_top_procs from './components/use_mem_top_procs';
import Chart from "chart.js/auto";


function App() {
  const chartStyle = {
    width: '40%', // Establece el ancho al 50% de la pantalla
    margin: '20px auto', // Centra horizontalmente y añade espacio entre las gráficas
    display: 'block',
  };

  const headerStyle = {
    textAlign: 'center',
    backgroundColor: '#f4f4f4',
    padding: '20px',
    marginBottom: '20px',
  };

  const ip = "192.168.203.131"
  const port = "9001"

  return (
    <>
      {/* Header con información importante */}
      <header style={headerStyle}>
        <h1>Dashboard de Recursos del Sistema</h1>
        <p>Información importante sobre el estado actual del sistema: uso de memoria, fallos de páginas, uso de swap, uso de paginas y procesos que más memoria consumen</p>
      </header>

      <div style={chartStyle}>
        <h2>Uso de Memoria Física</h2>
        <Use_mem  ip={ip} port={port} />
      </div>

      <div style={chartStyle}>
        <h2>Uso de Memoria a lo largo del tiempo</h2>
        <Use_swap ip={ip} port={port}/>
      </div>

      <div style={chartStyle}>
        <h2>Fallos de Páginas</h2>
        <Fault_pages ip={ip} port={port}/>
      </div>

      <div style={chartStyle}>
        <h2>Páginas Activas e Inactivas</h2>
        <Use_pages ip={ip} port={port}/>
      </div>

      <div style={chartStyle}>
        <h2>Top 5 Procesos con Mayor Uso de Memoria</h2>
        <Use_mem_top_procs ip={ip} port={port}/>
      </div>



      
    </>
  );
}

export default App;
