# HTTP BROKER IN C
Um servidor http implementado em C tanto para MacOs/FreeBSD e para Linux, utilizando um sistemas de eventos (kqueue, epoll), permitindo um desempenho alto e eficiente por processo.

## Funcionalidades
O objectivo do servidor é realizar uma comunicação direta com a base de dados utilizando   o formato de JSON:API, permitindo assim execução das tarefas crud na base de dados. Implementado as regras de formatação da JSON:API.

Também permitirá a execução de logica propria, mas ainda analisar arquitectura, se sera executado internamente no processo do servidor ou utilizando o serviço beanstalk para a execução de jobs.

Possuindo uma das partes mais importantes a segurança nos pedidos http, validando se a rota e valida e permite acesso, e se o utilizador que esta aceder possui permissão de acesso.

## JSON:API
Na lógica de json api, sera disponibilizado os seguintes parâmetros:
- resource
- resource_id
- includes
- page
- sort
- filter (custom sql injection)

No body necessita de indicar o seguintes atributos:
- data
  - type
  - id
 - attributes
  - attributes of entity
 - relationship
  - resource
   - data (can be oject if only one, or array more than one)
    - attributes of entity

