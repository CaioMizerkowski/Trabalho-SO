# Trabalho de Sistemas Operacionais

O trabalho a seguir foi realizado conforme a supervisão do prof. Pedroso na matéria TE355, Sistemas Operacionais Embarcados, no qual o objetivo é aprofundar o entendimento das necessidades do uso de semáforos em sistemas multithread. Para tal foi entregue um servidor em estado parcialmente funcional para ter sua implementação finalizada com a adição de funções para o protocolo de comunicação estabelecido entre o servidor e o cliente. O protocolo visa controlar as alterações, escritas e gravações em um array de ditados e permitir a leitura dos ditados pelo cliente.

O cliente também teve que ser finalizado, com a implementação de funções especificas de forma a automatizar o envio de requisições ao servidor para permitir a consulta por múltiplos clientes, forçando a necessidade do uso de semáforos. Sendo a comunicação entre o servidor e o cliente feita através de sockets deixamos previamente configurados no código usado como base.

## Introdução

Devido as dificuldades de se compartilhar recursos entre processos, a estratégia da utilização de threads foi implementada tanto no ambiente windows como nos ambientes unix-like (Linux, Mac, etc). O uso de threads permite com que recursos definidos como globais no escopo do programa sejam compartilhados, permitindo com que várias tarefas acessem os mesmos recursos. No entanto, essa estratégia cria a necessidade de um processo de sincronização entre as threads de forma a que duas ou mais tarefas não utilizem o mesmo recurso simultaneamente.

Para resolver esse problema foi implementado o uso de semáforos, que são estruturas de controle que permitem que duas ou mais threads possam acessar recursos compartilhados de forma a evitar que este recurso seja utilizado por duas ou mais threads simultaneamente. Os semáforos, ao contrário de outras tentativas de sincronizar as threads como por meio de flags e de esperas ocupadas, se utilizam de recursos disponibilizados pelo hardware e pelo sistema operacional para bloquearem a execução de uma thread, que solicita um recurso ocupado, até que o recurso seja liberado pela thread que o solicitou.

Isso é extremamente relevante em sistemas do tipo cliente-servidor, no qual um servidor deve atender diversos clientes e cuja uma das estratégias para seu funcionamento é subir uma thread para cada cliente, de forma que os recursos utilizados por todas estas threads devem ser protegidos de forma a evitar que estes sejam utilizados por duas ou mais threads simultaneamente. Um exemplo prático disso é a proteção do array de ditados do problema proposto, que deve ser protegido de escritas e deleções simultâneas de forma a não existirem furos em seu conteúdo, como espaços vazios que não deveriam estar, duplicação de informações ou uma diferença entre o número de ditados realmente presente no arquivo e o número presente na variável que controla o número de ditados a serem gravados ao final do processo.

A comunicação entre o cliente e o servidor também trás outras dificuldades de sincronização que também necessitam do uso de semáforos para a correção através da proteção de regiões críticas do código. Neste caso, é a proteção do identificador do socket que deve ser passado para dentro da thread correspondente ao atendimento daquela conexão. Como o valor da thread é passado como um ponteiro para o valor, que então deve ser enviado para dentro da thread e salvo em uma variável do tipo int de escopo local. Durante todo este processo, existe a possibilidade da execução da thread ser preempetadas e outra thread assumir o local, modificando indevidamente o valor do socket e causando uma falha de comunicação entre o servidor e o cliente. Sendo que enquanto um cliente fica esperando por respostas e envios de uma thread que não existe, outro cliente passa a enviar para duas ou mais threads sem nenhuma garantia sobre qual thread vai atender os seus envios e em qual momento.

## Servidor

O código do servidor recebido foi expandido de forma a adicionar uma serie de funções, com algumas modificações na main tendo sido realizadas e diversas modificações na função responsável por atender a conexão de cada cliente, através da criação de uma thread que inicia esta função.

### Main

Dois semáforos são iniciados na main, chamados de *m* e *m2*, os dois iniciados com o valor de 1 e devendo ser usados somente para a criação de regiões de exclusão mutua, sendo estes semáforos também são chamados de mutex. A principal modificação na main se deu através do uso do segundo destes dois semáforos (*m2*) para proteger a região crítica dentro do loop que recebe as requisições dos clientes para se conectarem com o servidor através do socket disponibilizado. Neste trecho de código, o semáforo somente é submetido a operação **Down**, pois o fim de sua região crítica só ocorre dentro da thread após o valor do socket ter sido finalmente fixado na variável de escopo local. No código do servidor apresentado em anexo, essa região crítica se inicia na linha 455 dentro da função main e termina na linha 125, dentro da função atenderConexão, com a operação **Up** sendo realizada no semáforo.

Em pseudo código, podemos representar essa proteção de exclusão mútua da seguinte maneira:
PSEUDOCÓDIGO AQUI

### Atende Conexão

Na função atendeConexão, que deve existir idealmente numa thread separada para cada cliente que esteja se comunicando com o servidor, foram conferidas as regiões de exclusão mútua das funcionalidades já existentes e adicionadas novas funcionalidades, sempre observando quais são as regiões de exclusão mútua presentes nas mesmas.

Todas as funcionalidades esperadas foram implementadas com sucesso, adicionalmente uma funcionalidade de *HELP* foi também inserida para facilitar o debug na comunicação interativa com o servidor através do cliente. As funcionalidades e suas regiões de exclusão mutuas são as seguintes:

--Adicionar tabela de funcionalidades e regiões, com observações.

Dando-se enfase as funções de auxilio criadas para as funcionalidades mais complexas.

A função *remove_element* lida com a remoção do elemento e a eliminação de elementos vazios no meio do array de ditados, movendo todos os ditados com um índice maior do que o ditado excluído uma posição para trás e decrementando o contador de ditados. Toda essa operação ocorrendo dentro de uma região crítica.

A função de *busca* itera sobre todos os ditados, buscando aquele que possui como uma substring o termo buscado. Também estando inteiramente dentro da região crítica, pois itera sobre o array de ditados (msg).

A função *palavras-d* é usada tanto na contagem de palavras individuais dos ditados, como na contagem de palavras de todos os ditados, em ambos os casos dentro das regiões demarcadas para exclusão mútua. A diferença entre ambos os casos é que no segundo uma variável acumula os resultados retornados pela função, que é iterada sobre todos os ditados em um laço for. Enquanto que no primeiro caso não existe está iteração e somente um índice é consultado.

A função GravaDitado é uma adaptação da função LeDitado, disponibilizada pelo professor, no qual ao invés de se fazer a leitura, é feita a escrita em um arquivo especificado. A função LeDitado foi alterada para receber como parâmetro o nome do arquivo que possui os ditados, funcionalidade também presente na função GravaDitado, que recebe como parâmetro o nome do arquivo.

Para avisar ao cliente que a função foi corretamente executada, o servidor enviar um sinal de *ACK* ao final de cada execução com sucesso.

## Cliente

O cliente foi implementado através da expansão do código recebido como base, com a adição de duas threads, uma para receber os dados do servidor e outra para enviar os dados ao servidor, estando elas sincronizadas através do uso de um semáforo e com a opção de executar elas de modo interativa e não iterativo, enviando finitas vezes um comando para o servidor dentro de um loop sincronizado com a recepção das respostas.

### Main m

A função main do servidor é responsável pela conexão do programa através do socket, cujo código para o funcionamento foi disponibilizado no modelo. O semáforo para sincronização também é iniciado na mesma e um argumento extra foi adicionado a chamada da main. Este argumento é responsável por definir o modo de operação da cliente, se será interativo (0 ou vazio), executará deleções (1), rotações (2), substituições (3), leituras aleatórias (4) ou gravará os resultados (5).

### Envia Dados

A função enviaDados é responsável pelo envio de mensagens ao servidor, estando a thread da mesma sincronizada com a thread da função responsável por receber a resposta do servidor, essa sincronização ocorrendo por meio de um semáforo compartilhado. Esse semáforo só é usado nos casos de automação e seu funcionamento é ativado por meio de uma variável lock de escopo global, estando desabilitado o semáforo no modo interativo.

Ela fica desativada no modo interativo, pois a velocidade de envio dos comandos é menor e a sequência dos mesmos não determinística. Enquanto que nos casos automatizados, a velocidade de envio de comandos é muito rápida e a sequência totalmente determinada previamente.

Nos casos não interativos, o travamento ocorre pela operação **Down** aplicada ao semáforo compartilhado entre as threads, esta operação acontece a cada iteração do loop. Ficando a thread responsável por enviar os dados em espera até que servidor retorne um sinal de *ACK*.

No primeiro caso de execução automatizada de comandos, o caso da deleção, esta operação é repetida 100 vezes nas últimas posições do array de ditados.

No caso seguinte, o de rotação, esta operação é realizada 500 vezes, incrementando sempre o índice do ditado que será afetado.

No caso da execução das operações de *replace*, esse comando também está setado para ser executado 500 vezes, alterando o ditado afetado e jogando um texto informativo no lugar.

No quarto caso, leituras aleatórias são executadas 500 vezes pela thread através do comando *GETR*, usado principalmente para ocupar o servidor, já que está operação não faz alterações nos ditados.

A última função, é responsável por salvar os ditados daquela thread utilizando o número do socket como parte do nome do arquivo.

### Recebe Dados

Estando responsável por receber os dados do servidor ao cliente, a função recebeDados é sincronizada a thread de enviar dados, nos casos automatizados, através de um semáforo. Sendo nestes casos ela a responsável por executar a operação **Up** no semáforo de tamanho um.

Para acomodar os casos interativos e não interativos, um lock global foi aplicado. Sendo o mesmo responsável por definir se a operação **Up** será ou não executada a cada vez que o cliente receber o sinal de *ACK* do servidor.

## Metodologia

### Primeira tarefa

Para a comprovação da eficiência do uso de semáforos como solução para o problema de regiões de mútua exclusão e de sincronização, sejam de threads ou do cliente com o servidor, foram feitos dois caso de teste.

O primeiro caso foi o uso de um servidor com todos os semáforos comentados em código, no qual não existe portanto a sincronização entre os as threads que atendem os clientes e no qual não existem regiões protegidas do código. Se esperando que neste caso exista uma perda de sincronização entre os clientes e as threads, assim como a mudança durante os momentos de leitura e escrita das várias globais, causando os mais diversos problemas.

O segundo caso é de um servidor com os semáforos colocados em duas regiões corretas, tanto para a sincronização das threads com os clientes, como para a proteção das regiões críticas, nas quais se alteram as variáveis de escopo global.

Em ambos os casos, 3 diferentes conjuntos de operações foram realizadas de forma a coletar os problemas realizados e a situação esperada. No primeiro (caso rotate) deles foi executado 102 clientes com operações de rotate (500 alterações por cliente), esperando-se 51000 operações. No segundo (caso replace) foram executados 10 clientes executando cada um 500 operações de replace, esperando-se a contagem de 5000 alterações. No terceiro (caso misto) foram realizadas 6 operações de cada um dos tipos: getr, replace, rotate e del, totalizando 24 clientes distintos e esperando-se um total de 6600 alterações e 3000 leituras.

### Segunda tarefa

Para o segundo item pedido ao relatório, foi separado a execução do servidor e do cliente em máquinas diferentes e sobrecarregado o CPU da máquina do servidor. Sendo feita a mudança em várias configurações do sistema operacional, para testar o efeitos destas configurações na velocidade de execução. Estes testes foram realizados primeiramente no modo padrão de compartilhamento de tempo do sistema linux (SCHED_OTHER) e após isso no modo de execução em tempo real (SCHED_RR) do mesmo sistema.

## Resultados

### Primeira tarefa

#### Servidor sem semáforos

No caso do servidor sem os semáforos foram encontrados resultados dispares em relação aos resultados esperados numa situação exitosa.

O mais visível destes foi a execução de forma incompleta do total de alterações esperadas. No primeiro caso analisado, somente 29275 dentre 51000 operações (57%) foram executadas, no segundo foram 2831 de 5000 (56%) e no terceiro apenas 2454 de 6600 operações (37%).

Também foram observados dois efeitos correlacionados, o de clientes que ficaram presos ao final da execução do programa, nunca terminando a sua execução, e o de sockets que acabaram sendo compartilhados por múltiplas threads. Indicando que certos clientes ficaram *órfãos*, não estando conectados diretamente com nenhuma das threads, enquanto que outros se comunicavam com diversas, perdendo-se a garantia das mensagens enviadas chegarem corretamente.

Os clientes presos foram 44 de 102 (43%) para o caso do rotate, 4 de 10 (40%) para o caso do replace e 12 de 24 (50%) para o misto.

Em todos os casos, diversos erros de protocolos ocorreram pela falta de sincronismo entre o cliente e o servidor. No qual o parâmetro era interpretado como o comando ao servidor, tendo outros casos no qual o comando do servidor era interpretado como um parâmetro.

Neste segundo caso, pode-se ver claramente isso ocorrer no arquivo com os ditados salvos após as operações, especialmente no caso do replace e no caso misto. No qual os ditados foram substituir algumas vezes pela palavra "REPLACE".

No caso do rotate e no caso misto, também foi visto o fenômeno de duplicação de linhas. Ocorrendo 17 duplicações no caso misto e 5 duplicações no caso do rotate.

#### Servidor com semáforos

No caso do semáforo nenhum dos problemas reportados foi encontrando. Nenhum cliente ficou preso, não ocorreu duplicação de linhas e todas as operações esperadas foram realizadas. Confirmando a solução dos semáforos como um método eficaz de sincronizar threads e de fazer a proteção de regiões de múltipla exclusão no código.

### Segunda tarefa

## Conclusão

### Primeira tarefa

O uso correto dos semáforos permitiu o bom funcionamento do servidor ao atender as requisições de múltiplos clientes de forma simultânea. Enquanto que na falta de semáforos, diversos problemas, alguns já esperados anteriormente e outros não, ocorreram. Impossibilitando o uso do servidor sem os semáforos de forma consistente, em especial durante ocasiões de estresse com alta demanda por parte de uma infinitude de clientes.

### Segunda tarefa
