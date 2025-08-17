Make sure Docker-Desktop is installed and running on your machine.

Steps to build docker image from Dockerfile:
1. move to the folder user_service
2. Run command: docker build -t user-service-img -f docker-files/Dockerfile .

Steps to run the docker image
1. Run command: docker run --rm -p 8001:8001 --name user-service-container user-service-img