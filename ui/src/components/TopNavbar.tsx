import React from 'react';
import Container from 'react-bootstrap/Container';
import { Navbar, Nav } from 'react-bootstrap';

const TopNavbar: React.FC = () => {
  return (
    <Navbar bg="dark" variant="dark">
      <Container>
        <Navbar.Brand href="/home">Taipei Station</Navbar.Brand>
        <Nav className="mr-auto">
          <Nav.Link href="/taxi">Taxi</Nav.Link>
        </Nav>
      </Container>
    </Navbar>
  );
};

export default TopNavbar;