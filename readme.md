# CORD RD

The project consists of four apps located in the `apps` directory. There is one Endpoint for the simple registration, one Endpoint for the normal registration, one lookup client as well as the resource_directory app. When the resource_directory app runs and is deployed on a feather board, the gcoap listener starts and listens for requests to handle them correctly. The Endpoint application represents an EP that sends a registration requests to the RD. Once this app is deployed on a feather board it starts discovering the local RD and sends a registration requests to it. The 
same applies to the epsim_endpoint app but this application triggers the simple registration process. The last one is a lookup client, which sends 
lookup queries to the RD application, based on how it is configured.  
