import { createRef, useCallback, useEffect, useState } from "react";
import { ToastContainer, toast } from 'react-toastify';
import { GoogleMap, Marker, useJsApiLoader } from '@react-google-maps/api';
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
  const [map, setMap] = useState<google.maps.Map | undefined>();
  const [lat, setLat] = useState(53.392046);
  const [lng, setLng] = useState(-1.507835);

  const updateData = useCallback(() => {
    fetch(url)
      .then(res => res.json())
      .then(setData)
      .catch(err => toast.error(err.message))
      .finally(() => setTimeout(updateData, 250));
  }, [data]);

  useEffect(() => {
    console.log("render");
    updateData();
  }, []);

  useEffect(() => {
    if (data?.lat && data?.lng) {
      setLat(data.lat);
      setLng(data.lng);
    }
  }, [data]);

  const { isLoaded } = useJsApiLoader({
    id: 'google-map-script',
    googleMapsApiKey: process.env.REACT_GOOGLE_MAPS_API_KEY || '',
  });


  const onLoad = useCallback(function callback(map: google.maps.Map) {
    console.log("onLoad");
    setMap(map);
  }, []);

  const onUnmount = useCallback(function callback(map: google.maps.Map) {
    setMap(undefined);
  }, []);

  function renderData(data: any) {
    return Object.keys(data).map(key => <div>
      <span><b>{key}:</b> {data[key]}</span>
    </div>);
  }


  return (
    <div className="App">
      <ToastContainer />
      <header className="App-header">
      </header>

      <div className="map">
        {
          isLoaded ? (
            <GoogleMap
              mapContainerStyle={{ width: '100%', height: '100%' }}
              center={{ lat, lng }}
              zoom={18}
              onLoad={onLoad}
              onUnmount={onUnmount}
            >
              <>{ data ? <Marker position={{ lat: data.lat, lng: data.lng }} label={`${data.mph} mph`} title="Woof!" /> : null }</>
            </GoogleMap>
          ) : <></>
        }
      </div>
      <div className="data">
        <pre>
          { data ? renderData(data) : null }
        </pre>
      </div>
    </div>
  );
}
