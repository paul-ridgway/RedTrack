import { useEffect, useState } from "react";
import { ToastContainer, toast } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';

const url = 'http://192.168.123.1/gps';

export interface Data {
  lat: number;
  lng: number;
  day: number;
  month: number;
  year: number;
  hour: number;
  minute: number;
  second: number;
  centisecond: number;
  kmph: number;
  mph: number;
  mps: number;
  satellites: number;
}


export function App() {

  const [data, setData] = useState<Data | undefined>();

  function updateData() {
    fetch(url)
      .then(res => res.json())
      .then(setData)
      .catch(err => toast.error(err.message))
      .finally(() => setTimeout(updateData, 250));
  }

  useEffect(() => {
    updateData();
  }, []);

  return (
    <div className="App">
      <header className="App-header">
      </header>
      <pre>
        {JSON.stringify(data, null, 2)}
      </pre>
    </div>
  );
}

