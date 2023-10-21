import React from 'react';
import './App.css';
import Map from './components/Map';
import TopNavbar from './components/TopNavbar';
import Taxi from './components/Taxi';
// import { HashRouter as Routes, Route } from 'react-router-dom';
import {Navigate, useRoutes} from 'react-router-dom';

function App() {
  return (
    <div className="App bg-color">
      <TopNavbar />
      {useRoutes([
        {path: '/', element: <Map />},
        {path: 'taxi', element: <Taxi />},
        {path: 'taxi/:exitNumber', element: <Taxi />},
        {path: '*', element: <Navigate to="/" />}
      ])}
    </div>
  );
}

export default App;
