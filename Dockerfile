FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y build-essential make g++ libmysqlclient-dev && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN make relay_server

EXPOSE 8888

CMD ["./relay_server", "0.0.0.0", "8888"]
