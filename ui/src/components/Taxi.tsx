import { useState, useEffect, useCallback } from 'react';
import { Button, Card, CardGroup, Container, Row, Col } from 'react-bootstrap';
import { useParams } from 'react-router-dom';

export interface ExitData {
  exitPosition: number;
  taxiCount: number;
  passengerCount: number;
}

const Taxi: React.FC = () => {
  const car_label = [476, 478, 480, 566, 576, 706, 818, 830, 570, 865, 868, 655, 781, 875, 582, 610, 628, 657, 676, 735, 661, 662, 718, 758, 469, 512]
  const { exitNumber } = useParams();
  const [exitData, setExitData] = useState<ExitData[]>([
    {
      "exitPosition": 1,
      "taxiCount": 0,
      "passengerCount": 0
    },
    {
      "exitPosition": 2,
      "taxiCount": 0,
      "passengerCount": 0
    },
    {
      "exitPosition": 3,
      "taxiCount": 0,
      "passengerCount": 0
    },
    {
      "exitPosition": 4,
      "taxiCount": 0,
      "passengerCount": 0
    }
  ]);

  const [prevId, setPrevId] = useState(-1);
  const [carCountData, setCarCountData] = useState<number[]>([0, 0, 0, 0]);

  const requestSerialPort = useCallback(() => {
    (navigator as any).serial.requestPort().then((port: any) => {
      port.open({ baudRate: 115200 }).then(() => {
        const decoder = new TextDecoderStream();
        port.readable.pipeTo(decoder.writable);
        const reader = decoder.readable.getReader();
        let result = '';

        reader.read().then(function process({ value }): any {
          result += value;
          const endlIdx = result.indexOf('\n');
          if (endlIdx !== -1) {
            const s = result.slice(4, endlIdx);
            let data: any;
            try {
              data = JSON.parse(s);
              if (data.id >= 0 && data.id <= 3) {
                if (car_label.includes(data.label) === true && data.confident >= 25) {
                  setCarCountData((prev) => {
                    const updated = [...prev];
                    updated[data.id]++;
                    return updated;
                  });
                }
                if (prevId !== data.id) {
                  setPrevId(data.id);
                }
              }
            } catch (error) { }
            result = result.slice(endlIdx + 1, -1);
          }
          return reader.read().then(process);
        }).catch((error: any) => {
          alert(error);
        });
      }).catch((error: any) => {
        alert(error);
      });
    }).catch((error: any) => {
      alert(error);
    });
  }, []);



  useEffect(() => {
    const updatedExitData = exitData.map((exit, index) => ({
      ...exit,
      taxiCount: carCountData[index],
    }));

    setCarCountData((prev) => {
      const updated = [...prev];
      updated[prevId] = 0;
      return updated;
    });

    setExitData(updatedExitData);
  }, [prevId]);

  const sortedData = exitData.slice().sort((a, b) => {
    const diffA = a.taxiCount - a.passengerCount;
    const diffB = b.taxiCount - b.passengerCount;

    if (diffB !== diffA) {
      return diffB - diffA;
    } else {
      return b.taxiCount - a.taxiCount;
    }
  });

  return (
    <Container style={{ height: '100vh' }}>
      <Row className="align-items-center">
        <Col>
          <div></div>
        </Col>
        <Col>
          <h1 style={{ color: 'white', margin: '30px' }}>Taxi Page</h1>
        </Col>
        <Col>
          <Button variant="secondary" size="lg" onClick={requestSerialPort} className="float-right">
            Get Data
          </Button>
        </Col>
      </Row>
      {sortedData.map((exit, index) => (
        <Row className="justify-content-center" style={{ height: '17vh', marginBottom: '30px' }} key={exit.exitPosition}>
          {/* <CardGroup> */}
          <div className='col-4' style={{ paddingRight: '0pt' }}>
            <Card
              key={exit.exitPosition}
              bg={exitNumber === exit.exitPosition.toString() ? 'secondary' : 'dark'}
              text='white'
              // border={exitNumber === exit.exitPosition.toString() ? 'secondary' : 'dark'}
              style={{ borderTopLeftRadius: '25pt', borderBottomLeftRadius: '25pt',  borderTopRightRadius: '0pt', borderBottomRightRadius: '0pt' }}
              className='mb-4  h-100'
            >
              <Card.Body className="d-flex align-items-center justify-content-center">
                <Card.Text style={{ fontSize: '30px' }}>
                  {(() => {
                    switch (exit.exitPosition) {
                      case 1:
                        return "東1門";
                      case 2:
                        return "南1門";
                      case 3:
                        return "西1門";
                      case 4:
                        return "北1門";
                      default:
                        return "Exit " + exit.exitPosition;
                    }
                  })()}
                </Card.Text>
              </Card.Body>
            </Card>
          </div>
          <div className='col-8' style={{ paddingLeft: '0pt' }}>
            <Card
              key={exit.exitPosition + 4}
              bg={exitNumber === exit.exitPosition.toString() ? 'secondary' : 'dark'}
              text='white'
              // border={exitNumber === exit.exitPosition.toString() ? 'secondary' : 'dark'}
              style={{ borderTopLeftRadius: '0pt', borderBottomLeftRadius: '0pt', borderTopRightRadius: '25pt', borderBottomRightRadius: '25pt' }}
              className='h-100 mb-4'
            >
              <Card.Body className="d-flex align-items-center justify-content-center">
                <Card.Text style={{ fontSize: '20px' }}>
                  Taxi Count: {exit.taxiCount}
                  <br />
                  Passenger Count: {exit.passengerCount}
                </Card.Text>
              </Card.Body>
            </Card>
          </div>
          {/* </CardGroup> */}
        </Row>
      ))
      }
    </Container >
  );
}

export default Taxi;
