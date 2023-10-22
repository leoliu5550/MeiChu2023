import React, { useState, useEffect } from 'react';
import { Link } from 'react-router-dom'; // Import Link from react-router-dom
import './Map.css';
import ImageMarker, { Marker, MarkerComponentProps } from 'react-image-marker';
import { Tooltip } from 'react-tooltip'

const Map: React.FC = () => {
  const [selectedExit, setSelectedExit] = useState(0);

  const handleExitClick = (exitNumber: number) => {
    setSelectedExit(exitNumber);
  };

  const [markers, setMarkers] = useState<Array<Marker>>([
    { top: 21, left: 85 },
    { top: 80, left: 80 },
    { top: 23, left: 4 },
    { top: 7, left: 66 }
  ]);

  const CustomMarker = (props: MarkerComponentProps) => {
    const { itemNumber } = props;
    let exitText;
    switch (+itemNumber + 1) {
      case 1:
        exitText = "Exit 東1門";
        break;
      case 2:
        exitText = "Exit 南1門";
        break;
      case 3:
        exitText = "Exit 西1門";
        break;
      case 4:
        exitText = "Exit 北1門";
        break;
      default:
        exitText = "Exit";
    }

    return (
      <>
        <Link to={`/taxi/${+itemNumber + 1}`}>
          <div
            draggable="true"
            onDrop={(event: any) => {
              console.log(event);
            }}
            data-tooltip-id={`${itemNumber}`}
            data-tooltip-content={`${exitText}: click to check taxi and passenger count!`}
            className={`custom-marker marker-color`}
          ></div>
          <Tooltip
            id={`${itemNumber}`}
            style={{ backgroundColor: "rgb(20, 20, 20)", color: "#FFF" }}
          />
        </Link>
      </>
    );
  };

  return (
    <div className="map-container">
      <ImageMarker
        src="taipei_station_floor_plan.jpg"
        markers={markers}
        markerComponent={CustomMarker}
      />
    </div>
  );
};

export default Map;
