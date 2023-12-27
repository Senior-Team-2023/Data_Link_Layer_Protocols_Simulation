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
       
        nodes[2]:Node;
    connections:
        coordinator.out[0] --> nodes[0].in;
        coordinator.out[1] --> nodes[1].in;
        nodes[0].ino <--> nodes[1].ino;




}

