FROM ubuntu:22.04

RUN apt-get update && apt-get install -y g++ && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN g++ server.cpp -o app -O2 -pthread

EXPOSE 8080

CMD ["./app"]
