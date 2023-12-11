package networks_project;

@license(LGPL);
//
// TODO documentation
//
network Network
{
    @display("bgb=673,352");
    submodules:
        coordinator: Coordinator {
            @display("p=337,206");
        }
        node0: Node {
            @display("p=101,90");
        }
        node1: Node {
            @display("p=556,101");
        }
    connections:
        coordinator.out[0] --> node0.in;
        coordinator.out[1] --> node1.in;
        node0.ino <--> node1.ino;
}

