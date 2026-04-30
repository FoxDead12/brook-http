## BROOK

Servidor HTTP de alta performance e eficiente tendo como objetivo a submissão de jobs. A arquitetura deste servidor foi baseada pelo nginx, fazendo uso de eventos dos files descriptors e a implementação de worker processes. O servidor deve ser usado com o nginx a fazer proxy pois não serve ficheiros estaticos e nem possui ecriptção SSL e apenas aceita conexões localhost.

A sua função é conseguir submeter jobs para um serviço de queue beanstalkd
