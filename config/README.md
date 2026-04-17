## Gatekeeper File Struct
O ficheiro **Gatekeeper** é responsável pela gestão e validação das rotas disponíveis no servidor. Estruturado em formato JSON, este ficheiro armazena um array de objetos que define o comportamento de cada endpoint.

### Rotas Públicas
As rotas classificadas como **públicas** não requerem uma sessão ativa ou autenticação de utilizador para serem executadas. Isto significa que qualquer cliente pode submeter um _job_ para processamento através destas rotas.

**Exemplo de configuração (Rota Pública):**
```
{
	"method":  ["POST"],
	"route":  "/url/exemplo",
	"job":  {
		"tube":  "nome-do-tubo"
	}
}
```

### Rotas protegidas
As rotas **protegidas** exigem que a ligação inclua uma sessão ativa (normalmente através de um _token_ de acesso). Para que o pedido seja processado, o sistema valida dois critérios essenciais:
-   **Validade da Sessão:** O _token_ deve ser autêntico e estar dentro do prazo de validade.
-   **Match de Permissões:** O nível de acesso do utilizador deve corresponder às permissões exigidas pela rota específica.
```
{
	"method":  ["POS"],
	"route":  "/url/exemplo",
	"role_mask":  "0x1", --> novo campo onde definimos as roles em hexadecimal
	"job":  {
		"tube":  "nome-do-tubo"
	}
}
```

## Gatekeeper Struct File to Multiple Apps
Esta configuração permite que o mesmo servidor valide e processe pedidos provenientes de diferentes aplicações web, garantindo a separação de contextos e a atribuição correta de tarefas.

**Injeção de Atributos Estáticos no Job**
O Gatekeeper permite a anexação automática de atributos fixos ao _job_ antes da sua submissão para o tubo. Esta funcionalidade é essencial para identificar a origem do pedido ou transmitir metadados constantes de forma transparente para o cliente.

Esta capacidade torna-se **fundamental** num ecossistema multi-app, uma vez que o servidor utiliza um **sistema de autenticação partilhado** por todas as aplicações. Através da injeção destes atributos, o servidor consegue distinguir e segmentar o processamento de tarefas, mesmo quando estas provêm de fontes que partilham a mesma infraestrutura de segurança.

Exemplo de atributos estáticos:
```
{
	"method":  ["POST"],
	"route":  "/url/exemplo",
	"job":  {
		"tube":  "nome-do-tubo",
		"attributes": {
			"app": "web-app-name",
			"key": "value"
		}
	}
}
```

**Proteção de Rotas por Aplicação**
Para cenários onde a segurança deve ser isolada por contexto, o ficheiro permite validar o acesso com base na aplicação de origem. Nestes casos, o sistema não valida apenas o _token_, mas também se a sessão pertence especificamente à aplicação definida no campo `role_app`.

Exemplo de rota protegida por App e Permissão:
```
{
	"method":  ["POS"],
	"route":  "/url/exemplo",
	"role_mask":  "0x1",
	"role_app": "web-app-name", --> novo campo para validar se a sessão contem este campo com o mesmo valor
	"job":  {
		"tube":  "nome-do-tubo"
	}
}
```
