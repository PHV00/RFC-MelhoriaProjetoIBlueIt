**Engenharia de Software – Católica SC**

---

# Identificação

- **Título do Projeto:**  
  Melhorias no projeto I Blue It.

- **Linha de Projeto (Direction):**  
  IA / IoT / Dados

- **Autor:**  
  Pedro Henrique Vitoreti

- **Data da Proposta:**  
  12/04/2026

- **Versão:**  
  1.0

---

# 1. Visão do Produto e Impacto (O Problema)
  
Este projeto tem como fim melhorar o já consolidado jogo sério I Blue It, um software desenvolvido pela Universidade do Estado de Santa Catarina (UDESC) com o princípio de auxiliar na fisioterapia de pacientes com problemas respiratórios, isso por meio da gamificação dos seus exercícios e análises dos resultados. Assim o projeto pretende aplicar as melhorias que foram encontradas, tanto técnicas, como de demandas da comunidade que já o utiliza.

---

## 1.1 Contexto e Problema

A reabilitação respiratória é um processo terapêutico onde pacientes com disfunções respiratórias, usam de uma abordagem de longo prazo, sistemática e repetitiva, o que pode levar a reduzir a adesão do paciente ao tratamento. Nesse contexto os jogos sérios surgem como uma forma de tornar os exercícios mais motivadores, lúdicos e engajadores aos pacientes.

Nesse âmbito o software já desenvolvido pela iniciativa de pesquisa da Udesc, I Blue It gamifica o tratamento e retorna dados úteis ao profissional da saúde que aplica o tratamento sobre paciente. O ecossistema do I Blue It pode ser dividido em três áreas distintas sendo elas os jogos, onde possuímos o jogo principal de mesmo nome do projeto e mais alguns minigames, os dispositivos IoT chamados PITACO e Mano-BD responsáveis pela captura dos dados de respiração do paciente e o dashboard responsável por fornecer ao profissional da saúde dados relevantes dos seus pacientes. 

O jogo principal consiste em um jogo de plataforma, onde o usuário por meio de sua respiração no dispositivo PITACO movimenta verticalmente o personagem para desviar dos obstáculos, inspirando para subir e expirando para descer assim movimentando o personagem "Blue" (o golfinho), além disso possui outros minigames com mesmo conceito para exercitar o paciente, sendo os dados destes exercícios utilizado pelo profissional de saúde para regular e avaliar o estado do paciente.

Assim a solução que já está na versão 5.0 já contempla calibração, histórico do jogador, armazenamento de dados e customização de parâmetros por profissionais.

<img width="675" height="318" alt="image" src="https://github.com/user-attachments/assets/27040498-d538-4f3c-86c5-6c2aa9d87c31" />

[Imagem do jogo I Blue It da versão 2.0]

<img width="676" height="350" alt="image" src="https://github.com/user-attachments/assets/560cfbed-2ad8-4cab-9aac-4f24ad650252" />

[Imagem dos dispositivos IoT]

link auxiliar para mais informações sobre o software: https://udescmove2learn.wordpress.com/2018/04/26/i-blue-it/

Porém mesmo com todas estas versões e melhorias, foram identificadas algumas lacunas, percebeu-se um risco em potencial ao paciente, dado que os dados capturados atualmente não contemplam e nem fornecem em sua totalidade o necessario para um ajuste dinamico da dificuldade de maneira totalmente confiavel assim havendo o eventual risco de ou subdimencionar a dificuldade e comprometendo a terapia em curso, ou hiperdimencionar o exercicio, podendo acabar em casos extremos com desmaios, tonturas ou fadiga extrema por parte do paciente.

O que nos leva à decisão de implementar o módulo novo no IoT adicionando ao projeto, no qual sua adição acarreta em mudanças e ajustes diretos aos módulos de dashboards médicos e IA de ajuste de dificuldade ao novo componente.

---

## 1.2 Origem da Demanda e Evidências

O projeto se origina em uma iniciativa do (LARVA) Laboratory for Research on Visual Applications da UDESC em colaboração com profissionais da área de fisioterapia respiratória e áreas correlatas, com o objetivo de atender a problemáticas reais do domínio clínico. Sendo assim desde o princípio o sistema tem como foco atender as necessidades de ferramentas que auxiliem no processo de atendimento médico e tratamento de pacientes com problemas respiratórios.

O programa que foi concebido em 2018 já se utilizava de mais de 80 profissionais da saúde (fisioterapeutas, médicos pneumologistas, fisioterapeutas respiratórios, neurologistas) os quais atuavam como conselheiros com a finalidade de garantir a efetividade e utilidade da ferramenta ao longo do próprio desenvolvimento e que ao final o avaliaram, obtendo um resultado muito satisfatório uma nota 4.1 de 5, demonstrando grande satisfação por parte dos envolvidos. [2018 - Santos - I Blue It Um Jogo Sério para auxiliar na Reabilitação Respiratória]

Desde sua fase inicial até agora houveram diversas melhorias em cada uma das versões, onde podemos montar um histórico de versões de maneira resumida:

- versão 01 - I Blue It / Pitaco (2018):
  - Criação do dispositivo Pitaco (responsável por medir fluxo de ar) e do jogo "Blue"
- versão 02 - I Blue It / ManoBD (2019):
  - Adição de minijogos do Copo d’água e Bolo de Aniversário e concepção do dispositivo MonoDB (responsável por medir a pressão do ar)
- versão 03 - I Blue It / Health InfoCharts (2020):
  - Adição de módulo de análise clínica do paciente (Histórico e resultados dos jogos mostrados e armazenados)
- versão 04 - I Blue It / Multimodal (2020):
  - Incorporação de arquitetura Multimodal 123-SGR o que permitiu a incorporação de dispositivos distintos como Pitaco e ManoBD
- versão 04 - I Blue It / Multimodal redesign (2023):
  - Redesign para a incorporação de IA ao projeto
- versão 05 - I Blue It / Flow Psicofisiológico (2024):
  - Incorporação de Flow Psicofisiológico (busca equilibrar a parte motivadora psíquica, com a parte fisiológica do paciente) e IA ao projeto com o fim de controlar o Flow Psicofisiológico

Neste contexto este projeto de melhoria tem como ponto de partida dar continuidade no aprimoramento de uma ferramenta que auxilia a comunidade médica na recuperação de pacientes com problemas respiratórios, por meio de uma bolsa de Iniciação Científica adjunto a Udesc, tratando as lacunas encontradas e dando continuidade ao trabalho técnico acadêmico já desenvolvido.

## 1.3 Análise de Soluções Existentes (Benchmark)

**BubbleBreather**  
Uma pequena coleção de jogos/atividades para exercícios respiratórios focados apenas na recuperação de pneumonia. Pode ser acessado pelo GitHub público e possui uma demo web ativa, por mais que não haja updates desde 2020. Possui um escopo mais estreito sendo jogos feitos no próprio browser, depende de microfone, com foco em exercícios específicos e não apresenta uma camada clínica robusta, comparável a dashboard terapêutico ou equipamentos IoT terapêuticos multimodalidade com sensores dedicados.

<img width="1915" height="947" alt="image" src="https://github.com/user-attachments/assets/45f80ec5-01ce-4d98-9ae2-b7e611d33471" />

link do repositório : https://github.com/hcilab/BubbleBreather?tab=readme-ov-file

público-alvo : pessoas em recuperação de pneumonia

**PlayPhysio**

Uma iniciativa originada pela demanda de um pai cuja filha possui fibrose cística e necessitava realizar fisioterapia, porém o tratamento não era engajador o que dificultava a participação e o interesse da criança. Com isso em mente, decidiu-se criar uma plataforma gamificada para que sua filha pudesse realizar seus exercícios de maneira mais lúdica. Para fazer isso a plataforma acopla um equipamento IoT chamado PhysioPal ao equipamento terapêutico que ao se conectar via bluetooth no aparelho celular, registrará a pontuação no app. Não parece apresentar publicamente um ecossistema clínico tão robusto quanto o I Blue It, contendo dashboard terapêutico ou equipamentos IoT com sensores dedicados acoplados diretamente ao sistema, porém contém feedback em tempo real, e fornecimento detalhado de dados.

<img width="1922" height="952" alt="image" src="https://github.com/user-attachments/assets/0c27559d-9700-4069-ac29-197f41ecb948" />

links do projeto:
- https://www.jbs.cam.ac.uk/ventures/playphysio/
- https://play.physio/

público-alvo : pessoas com problemas respiratórios (com foco em crianças)

**ACPlus Respiratory Assessment + OmniFlow**

A empresa Accelerated Care Plus produz duas soluções no ramo de terapia respiratória sendo eles o ACPlus Respiratory Assessment uma solução voltada a ajudar na etapa de diagnóstico e decisão clínica de disfunções pulmonares, onde por meio de um dispositivo que captura os dados da respiração do paciente e os transmite a um iPad por meio do bluetooth, que por sua vez retorna em formato de prontuário com a documentação clínica necessária aos profissionais da saúde. A outra chamado OmniFlow, tem como foco o tratamento gamificado através de terapias pulmonares por meio de experiências interativas/gamificadas, onde o paciente realiza o tratamento através de um dispositivo espirômetro bluetooth que capta seus dados, que são utilizados tanto nos exercícios, quanto posteriormente permite a análise por um profissional de saúde. Entre suas limitações podemos destacar a exclusividade de implantação, pois atualmente os sistemas são voltados às regulamentações e normativas do seu país de origem Estados Unidos, o que limita a sua atuação preferencialmente apenas ao mercado estadunidense.

- OmniFlow
- <img width="1187" height="768" alt="image" src="https://github.com/user-attachments/assets/eef6fc3f-ce31-4bc5-8d05-24394b690f87" />

links dos produtos:
- https://acplus.com/acplus-respiratory-assessment-lp/
- https://acplus.com/technology/omniflow/
- https://acplus.com/blog/success-stories/omniflow-in-action-restoring-speech-confidence-and-connection/
  
público-alvo : pessoas com problemas respiratórios de modo amplo

---

### Comparação

| Solução | Pontos Fortes | Limitações |
|---|---|---|
| BubbleBreather |Permite a utilização do sistema em diversos ambientes (por ser em browser)|Não possui módulo clínico especializado|
| PlayPhysio |Permite tratamento gamificado, com especialização em exercícios lúdicos com foco infantojuvenil| Ecossistema clínico mais simplificado|
| OmniFlow |Permite o tratamento gamificado, e dados clínicos detalhados|Especificidade do sistema ao ambiente estadunidense|

---

### Diferencial do Projeto

Analisando os concorrentes elencados, podemos concluir que no campo das soluções respiratórias gamificadas elencadas se distribuem em três grandes grupos. O primeiro é o de soluções leves e acessíveis, como o BubbleBreather, que prioriza a simplicidade tecnológica e acesso rápido. O segundo é o das soluções orientadas ao engajamento familiar e à adesão de maneira lúdica, como o PlayPhysio. O terceiro é o das soluções com maior maturidade clínica e comercial, como o ACPlus Respiratory Assessment e o OmniFlow, fortemente ligado à atuação completa desde análise de desempenho do paciente em resposta ao tratamento, até ao tratamento gamificado e lúdico.

Porém podemos perceber que enquanto os dois primeiros concorrentes apresentem pontos fortes específicos e distintos, e que não cobrem todo o escopo já tratado pelo software I Blue It, o terceiro apresenta uma solução completa e semelhante ao software I Blue It, porém trazendo consigo uma diferença crucial, a especificidade ao ambiente estadunidense, o que impossibilita sua implementação de maneira simplificada em outros ambientes e sistemas.

Dado isto percebemos a lacuna nítida de uma ferramenta completa ao ambiente fisioterápico brasileiro, que abranja de maneira relevante tanto a gamificação dos exercícios fisioterápicos voltados à reabilitação respiratória, como a análise dos dados de desempenho do paciente, que é justamente a lacuna técnica que o I Blue It cobre.  

---

## 1.4 Público-Alvo

O sistema terá dois públicos principais:

Paciente em reabilitação respiratória: crianças e adultos que realizam exercícios terapêuticos respiratórios com acompanhamento profissional.

Profissional de saúde: fisioterapeutas respiratórios, fisioterapeutas clínicos, pneumologistas e demais especialistas que acompanham a sessão dos pacientes, definem parâmetros terapêuticos e analisam os resultados.

---

## 1.5 Objetivos do Projeto

### Objetivo Geral

O projeto se propõe por meio da incorporação de um novo módulo de captura de dados do paciente o Spo2 (sensor responsável pela captura da saturação sanguínea do paciente), oque acarreta em mudanças imediatas em partes já consolidadas do codigo, como o os modulos de dahboard medico (para fornecer mais um dado importante para tomada de decisão) e Ia de ajuste de dificuldade dinamica (que recebera mais um parametro para se adequar ao paciente em tratamento).

---

### Objetivos Específicos

Tendo em vista a problemática apresentada, este projeto tem como fim sanar as lacunas identificadas pela comunidade com fim de entregar uma plataforma mais completa e otimizada. Baseando-se nisso segue abaixo os objetivos a serem tratados:

- Integrar o sensor de Spo2 ao ecossistema do jogo para monitoramento fisiológico complementar.
- Atualizar o módulo de ajuste dinâmico de dificuldade para suportar o novo parametro de Saturação Sanguinea.
- Atualizar o dashboard clínico para exibir e correlacionar dados respiratórios, do Spo2 e de desempenho no jogo.
[2018 - Santos - I Blue It Um Jogo Sério para auxiliar na Reabilitação Respiratória (2).pdf](https://github.com/user-attachments/files/26648094/2018.-.Santos.-.I.Blue.It.Um.Jogo.Serio.para.auxiliar.na.Reabilitacao.Respiratoria.2.pdf)

---

## 1.6 Métricas de Sucesso (KPIs)

As Metricas de sucesso estipuladas são:

- Registro correto de sessões com dados respiratórios do novo componente SpO2 mantendo a velocidade e metricas do sistema atual.
- Ajuste dinâmico com delay de até 300ms na dificuldade durante a sessão, ao utilizar a nova IA que suporta todos os modulos IOT.
- Tempo de resposta do sistema inferior a 300 ms para feedback em gameplay.
- Adição dos dados obtidos pelo Sp02 ao dashboard apresentando todos os dados já existente e novos pertinentes, mantendo o tempo de resposta atual.

---

# 2. Engenharia de Requisitos

Este segmento define oque o sitema realizara para atender as melhorias no projeto I Blue It, tendo em vista a problematica, na qual os dados fornecidos pelo modelo atual não podem atender e nem garentir uma dificuldade automatica confiavel para evitar tanto a sua extrapolações quanto a sua atenuação ao nivel fisiologico suportado pelo paciente durante as sessões de exercicios, o projeto nesse sentido nessecita de um acrescimo , um novo modulo de monitoramento fisiologico com o fim de sanar está exata lacuna.

Para este fim é proposto que nesse novo modulo de monitoramento fisiologico possua um componente spo2(sensor de saturação sanguinea), oque acarreta em mudanças imediatas nos modulos de Dashboard clinico e de dificuldade ajustada por Inteligencia Artificial, sendo respectividamente acrecimos de informações clinicas para analise dos profissionais de saude, e parametro utilizado pela inteligencia artifical para estipular a dificuldade adequada dos exercicios.  

O projeto tem como premisa que o software I Blue It já possui uma  base funcional composta por jogo sério, dispositivos IoT para captura respiratória, armazenamento de dados, calibração por iteligencia artificial e acompanhamento por profissional.Sendo assim os requisitos estipulados abaixo descrevem as funcionalidades necessarios para a integração do novo modulo proposto, saturação sanguinea ao ecossitema existente. 

---

## 2.1 Personas
### Persona 1 — Ana Clara, paciente em reabilitação respiratória

**Contexto:**  
Ana Clara uma criança de 12 anos realiza sessões de fisioterapia respiratória, por possuir problemas respiratorios. Ela apresentava dificuldade em manter a constância dos exercícios, considerando o tratamento repetitivo e cansativo. Durante as sessões, passou a utilizar o jogo I Blue It com acompanhamento de um fisioterapeuta, oque apresetou repectiva melhora ao quadro.

**Objetivos:**  
- Proporsionar os exercícios respiratórios de uma forma mais lúdica e motivadora.
- Receber feedback visual durante a sessão de tratamento.
- Conseguir completar os niveis e desafios do jogo sem sentir excessivamente desconfortavel.
- Evoluir gradualmente a dificuldade, conforme sua melhora na capacidade respiratória.

**Principais dificuldades:**  
- Perde a motivação quando os exercícios são repetitivos.
- Em exercicios longos pode apresentar fadiga ou desconforto.
- Não pussue sensibilidade necessario para perceber quando está realizando esforço excessivo.
- Dependente da orientação de um profissional para realizar adequada dos exercícios propostos.

---

### Persona 2 — Dr. Marcos, fisioterapeuta respiratório

**Contexto:**  
Dr. Marcos atua em uma clínica voltada ao tatamento fisioterapico, na area respiratória onde acompanha pacientes com diferentes quadros clínicos. Ele se faz uso de ferramentas de apoio em vista de tornar os seus tratamentos mais motivador e ludico, assim o auxiliando na apicação do tratamento, acompanhamento e evolução dos pacientes ao longo das sessões.

**Objetivos:**  
- Configurar parâmetros para exercício seguindo o perfil clínico do paciente.
- Acompanhar os dados respiratórios e fisiológicos em tempo de execução da sessão.
- Visualizar histórico do paciente analisando seu desempenho e evolução.
- Identificar sinais de alerta tais como queda de saturação, fadiga ou baixo resultado ao exercício.
- Utilizar os dados obtidos e processados pelo sistema como apoio na tomada de decisão.

**Principais dificuldades:**  
- Necessita acompanhar simultaneamente a execução respiratória do paciente suas condições fisiológica e seu desempenho no jogo.
- Necessita de dados objetivamente organizados e claros para sua avaliação de evolução e tomada de decisão.
- Precisa evitar que o paciente seja submetido a esforços inadequados.
- Precisa ajustar a dificuldade do exercício de maneira a não interroper a sessão.

---

## 2.2 Casos de Uso Principais

Considerando o estado atual em que o software se encontra, com suas inumeras funcionalidades e aprimoramentos de cada versão, foram abstraidos as suas principais funções e caracteristicas para a criação de dois fluxos, consistindo em primasia uma abstração geral sobre o software I Blue It, e posteriormente a derivação de outro  para fins comparativos, contendo este as princiapis mudanças e impactos no fluxo que a melhoria se propẽm a realizar.

Para o atual estado do software foram elencado as seguintes funcionalidades:

### UC01 — Cadastrar/Selecionar Paciente

**Objetivo:** Identificar paciente que realizará a sessão terapêutica.

**Descrição:**
O sistema permite que o profissional de saúde cadastre o paciente caso seje novo, ou permite a seleção de um paciente previamente cadastrado.Essa funcionalidade tem como fim associar os pacientes as sessões do jogo e seus dados taus como calibrações, dados respiratórios, histórico e resultados.

---

### UC02 — Capturar dados respiratórios (via PITACO)

**Objetivo:** Receber os dados fisiológicos necessarios para a realização do jogo.

**Descrição:**
O sistema permite que o paciente por meio do dispositivo PITACO tenha seus dados fisiologicos estipulados e monitorados.Essa funcionalidade tem como fim capturar os dados necessarios para a realização de todas as operações do sistema.

---

### UC03 — Calibrar Respiração

**Objetivo:** Calibrar o sistema baseando-se na fisiológia do paciente.

**Descrição:**
O sistema permite que o paciente adjunto ao profissional de saúde calibrem o preparem o ambiente para a realização dos exercicios.Essa funcionalidade tem como objetivo adequar e preparar o software para realizar o exercicio ao paciente.

---

### UC04 — Configurar Sessão Terapêutica

**Objetivo:** Preparar a sessão com o exercicio  trapeutico adequado ao paciente.

**Descrição:**
O sistema permite que o profissional de saúde configure a sessão terapêutica a ser executada.Essa configuração se caracterizando por poder envolver desde a preparação do paciente, escolha do modo de jogo até a definição de parâmetros para a realização da atividade.

---

### UC05 — Executar Game Plataforma

**Objetivo:** Realizar o exercicio terapeutico com o jogo de plataforma

**Descrição:**
O sistema permite que o paciente ralize a sessão de exercicio teraupeutico do game de plataforma, no qual utiliza-se de sua respiração no instrumento PITACO para controlar um personagem verticalmente assim o fazendo desviar de obstaculos que aparecem para completar o nivel.Deste modo buscando exercitar o paciente de maneira lúdica. 

---

### UC06 — Executar Minigame

**Objetivo:** Realizar o exercicio terapeutico com os minijogos existentes 

**Descrição:**
O sistema permite que o paciente ralize a sessão de exercicio teraupeutico em um dos minigames existentes, no qual utiliza-se de sua respiração no instrumento PITACO para controlar as ações de cada nivel.Deste modo buscando exercitar o paciente de maneira lúdica. 

---

### UC07 — Personalizar Fases ou Níveis

**Objetivo:** Personalizar as fases para adeuquar ao perfil,fisiologia e evolução do paciente

**Descrição:**
O sistema permite que o profissional da saúde personalize as fazes para conter o tamanho e desafios adequados ao caso clinico do paciente.Esta funcionalidade permite a alteração dos niveis, fases e desafios do jogo de plataforma.

---

### UC08 — Acompanhar Sessão

**Objetivo:** Permitir supervisão por parte do profissional de saúde sobre a atividade terapêutica.

**Descrição:**
O sistema permite que o profissional de saúde acompanhe a realização da sessão terapêutica.Assim posibilitando observar o andamento e desempenho da execução do paciente, permitindo a tomadada de decisões durante ou breve após a sessão 

---

## UC09 — Ajustar Dificuldade Dinamicamente

**Objetivo:** Adequar a dificuldade do jogo à capacidade e ao desempenho do paciente.

**Descrição:**
O sistema ajusta a dificuldade da atividade em tempo de execução do exercicio terapeutico se adequando ao desempenho do paciente.Esse ajuste se caracteriza por envolver mudanças em parâmetros do jogo tais como obstaculos, velocidade, alvos ou nivel de desafio.

---

### UC10 — Registrar Histórico de Sessão

**Objetivo:** Armazenar dados da sessão fisioterapica para avaliação futura.

**Descrição:**
O sistema registra os dados gerados durante a sessão, tais como calibração, desempenho e dados respiratórios capturados.Esses dados constituindo o historico do paicente pedendo ser consultado posterioemente pelo profissional da saúde.

---

### UC11 — Consultar Dados / Relatórios

**Objetivo:** Disponibilizar informações obtidas anteroriormente para acompanhamento avanço clínico.

**Descrição:**
O sistema permite que o profissional da saúde consulte os dados e relatórios obtidos das sessões já realizadas.Essa funcionalidade auxilia o profissional na analise  da evolução do paicente e tomada de decisões eventuais envolvendo o tratamento. 

---

### UC12 — Interromper Sessão

**Objetivo:** Garantir soberania do profissional da saúde sobre da sessão terapêutica.

**Descrição:**
O sistema permite que o profissional da saúde interrompa a sessão quando achar necessário. Essa ocorrendo por decisão clínica esclusiva do proficional da saúde, baseada em sua experiencia e supervisão sobre o paciente em casos onde o paciente possa estar demosntrando desconforto, dificuldade na execução ou necessidade do encerramento da sessão de terapia.

---

Logo abaixo pode-se ver o diagrama que representa tais casos de uso e suas correlações: 

<img width="3404" height="4328" alt="image" src="https://github.com/user-attachments/assets/cef341ac-0b1d-4151-b5bc-1792789c4cc3" />

Já para a versão melhorada foram acresentados alguns casos, referentes ao novo modulo contendo o spo2, como pode-se ver abaixo:

### UC14 — Capturar Dados Respiratórios e SpO2 via PITACO Ampliado

**Objetivo:** Ampliar a coleta de dados fisiologicos do paciente, integrando no mesmo dispositivo PITACO captura de informações respiratórias e de oxigenação sanguínea.

**Descrição:**
O sistema permite capturar dados fisiologicos, por meio do PITACO ampliado, tanto os já utilizados dados respiratórios do paciente quanto os dados de saturação de oxigênio. Isso sendo feito por meio da adição do módulo Spo2 ao corpo do PITACO, com dispositivo passando a fornecer mais informações fisiológicas durante a sessão terapêutica.

---

### UC15 — Validar, Filtrar e Transformar Sinais Brutos

**Objetivo:** Garantir que os dados respiratórios capituados pelo SpO2 sejam consistentes, confiáveis e adequados para análise.

**Descrição:**
O sistema realiza a validação e tranformação dos dados brutos obtidos pelo Spo2 para dados como saturação sanguinea, frequência cardiaca e posição adequada do dispositivo, com a finalidade de fornecer tais dados processados para as outra partes interconectadas do sistema.Essa etapa permite validar se os dado estão sendo caputurados de maneira adequada e que estejam sendo retornados também de maneira adequada, para as demais partes.

---

### UC16 — Registrar Dados de Oxigenação

**Objetivo:** Armazenar os dados do Spo2 obtidos durante a sessão para fornecelos no acompanhamento clínico e análise posteriores.

**Descrição:**
O sistema registra os dados de oxigenação sanguinea obtidos durante a sessão pelo modulo Spo2.Esses dados passam a compor adjunto aos já existenteo o histórico do paciente, permitindo que o profissional da saúde em consultas posteriores tenha acesso a estas informações.

---

### UC17 — Emitir Alerta de Segurança

**Objetivo:** Alerta o profissional da saúde sobre possíveis riscos fisiológicos occoridos em tempo de execução da sessão terapêutica.

**Descrição:**
O sistema emite um alerta de segurança ao profissional da saúde quando indentifica que os sinais disiologicos do paciente indicam condições de risco ao mesmo, isso em durante a sessão terapêutica. Esses alertas podem estar relacionados a sinais incopativeis com a execução do exercicio como exesso ou falta de oxigenação , ou ainda a casos onde o dispositivo pode estar apresentando falahas na leitura.

---

### UC18 — Pausar/Interromper Sessão por Risco Fisiológico

**Objetivo:** Proteger o paciente durante a sessão, interompendo a atividade que desmontre situações de risco.

**Descrição:**
O sistema interrompe sessçoes em que forem identificados padrões anomalos ou de alto risco fisiologico ao paciente.Essa funcionalidade atua como medida de precaução e segurança, permitindo a pausa momentânea ou suspensão caso os sinais indiquem uma condição inadequada durante o exercicio.

****

Abaixo pode se ver o fluxo com a melhoria proposta:

<img width="3284" height="5128" alt="image" src="https://github.com/user-attachments/assets/9f41649e-9723-4fd5-8456-7e1de6a08285" />

---

## 2.3 Requisitos Funcionais (RF)

  Em vista dos trabalhos já realizados e documentados das versões passadas do software I Blue It, está proposta de melhoria não se atem aos requisitos funcionais da versão atual do software, e sim, apenas aos pertinentes a sua melhoria, nisto consistindo na integração do módulo de monitoramento de SpO₂ para apoiar a segurança fisiológica durante as sessões de reabilitação respiratória, cujoo o qual estão presentes logo abaixo.
  Outro adendo necessario, para diferenciar o local onde a melhoria será aplicada, do software já existente, foi nomeado a melhoria como modulo Spo2 sendo este o modulo responsável peloa captação, validação, correção e calculo de oxigenação e pulso cardiaco do paciente, e o software já existente sendo referido como software I Blue It, assim mostrando quais partes terão de fazer quais tarefas e suas respectivas interrações e responsabilidades.  

| ID | Requisito Funcional |
|----|---------------------|
| RF01 | O sistema deve permitir que o sensor Spo2 seja conectado ao sistema, sem interromper ou comprometer funcionalidades existentes. |
| RF02 | O sistema deve permitir que o sensor Spo2 envie dados durante a sessão a todos os modulos pertinentes do software I Blue It. |
| RF03 | O sistema deve permitir que o sensor Spo2 envie dados de frequência cardíaca, quando disponíveis. |
| RF04 | O sistema deve permitir que o software I Blue It receba dados do modulo Spo2 em tempo real. |
| RF05 | O sistema deve permitir que o modulo Spo2 valide o contato do sensor ao paciente. |
| RF06 | O sistema deve permitir que o modulo Spo2 valide a qualidade do sinal recebido. |
| RF07 | O sistema deve permitir que o modulo Spo2 ignore leituras inválidas de oxigenação. |
| RF08 | O sistema deve permitir que o sistema monitore a oxigenação do paciente durante a execução do jogo. |
| RF09 | O sistema deve permitir que o terapeuta configure o limite mínimo de oxigenação. |
| RF10 | O sistema deve permitir que o modulo Sp02 compare a oxigenação obtida, com o limite definido pelo terapeuta. |
| RF11 | O sistema deve permitir que o modulo Sp02 alerte o software I Blue It quando a oxigenação estiver abaixo do limite definido. |
| RF12 | O sistema deve permitir que o modulo Spo2 emita alerta quando o sensor estiver desconectado. |
| RF13 | O sistema deve permitir que o modulo Spo2 emita alerta quando o sensor estiver mal posicionado, pausando a sessão. |
| RF14 | O sistema deve permitir que o modulo Spo2 registre a série temporal de oxigenação da sessão no software I Blue It. |
| RF15 | O sistema deve permitir que o modulo Spo2 registre o menor valor de oxigenação da sessão. |
| RF16 | O sistema deve permitir que o modulo Spo2 registre eventos de queda de Spo₂. |
| RF17 | O sistema deve permitir que o registre eventos de falha ou perda de sinal do modulo Spo2. |
| RF18 | O sistema deve permitir que o modulo Spo2 recomende pausa ao terapeuta quando houver risco fisiológico ao paciente. |
| RF19 | O sistema deve permitir que o modulo Spo2 pause automaticamente a sessão no software I Blue It caso a oxigenação esteja em condição crítica. |
| RF20 | O sistema deve permitir que o terapeuta autorize a retomada da sessão após normalização da Spo2. |
| RF21 | O sistema deve permitir que o software I Blue It apresente o estado do monitoramento fisiológico na interface. |
| RF22 | O sistema deve permitir que o terapeuta visualize os dados do modulo Spo2 durante e após a sessão. |
| RF23 | O sistema deve permitir que o modulo Spo2 gere um resumo fisiológico da sessão para ser apresentado no software I Blue It. |
| RF24 | O sistema deve permitir a associação dos dados obtidos pelo modulo Spo2 aos dados já obtidos pelo software I Blue It durante a sessão terapeutica . |
| RF25 | O sistema deve permitir que o terapeuta consulte os registros fisiológicos de sessões anteriores. |
| RF26 | O sistema deve permitir que em caso de eventuais pausas os dados fisiologicos até então captados não sejam perdidos e possam ser recuperados . |

---

## 2.4 Requisitos Não Funcionais (RNF)]

Os requisitos não funcionais descritos nesta seção seguem o memso principio descrito nos Funcionais onde o foco para elencalos foi tendo em mente a solução e oque ela alterará no modelo atual, porém agora estipulando metricas qualitativas de desempenho, segurança, confiabilidade, usabilidade, compatibilidade e manutenção que devem ser atendidas pela melhoria proposta.

| ID | Categoria | Requisito Não Funcional |
|----|-----------|--------------------------|
| RNF01 | Compatibilidade | O módulo Spo2 deve ser integrado ao software I Blue It sem comprometer as funcionalidades já existentes no software ou hardware. |
| RNF02 | Modularidade | O módulo Spo2 deve ser desenvolvido de forma independente, permitindo sua manutenção sem necessidade de alterar partes centrais do software I Blue It. |
| RNF03 | Interoperabilidade | O módulo Spo2 deve se comunicar com o software I Blue It por meio de um formato de dados definido e compreensível pelos dois componentes. |
| RNF04 | Desempenho | O módulo Spo2 deve atualizar os dados de oxigenação e frequência cardíaca em tempo compatível com o acompanhamento da sessão terapêutica. |
| RNF05 | Tempo de resposta | O módulo Spo2 deve emitir alertas de risco fisiológico em tempo suficiente para que o terapeuta possa intervir durante a sessão. |
| RNF06 | Confiabilidade | O módulo Spo2 deve evitar o uso de leituras inválidas, instáveis ou incompatíveis com um sinal fisiológico confiável. |
| RNF07 | Robustez | A perda de conexão com o sensor Spo2 não deve causar travamento ou encerramento inesperado do software I Blue It, mas deve gerar alertas e avisos. |
| RNF08 | Segurança fisiológica | O módulo Spo2 deve priorizar a segurança do paciente em relação à continuidade da sessão no jogo. |
| RNF09 | Segurança operacional | O software I Blue It deve ser capaz de interromper, pausar ou sinalizar risco quando o módulo Spo2 indicar condição fisiológica crítica. |
| RNF10 | Usabilidade | O estado do módulo Spo2 deve ser apresentado de forma clara ao terapeuta, indicando se o sensor está conectado, desconectado, válido, inválido ou em alerta. |
| RNF11 | Clareza de interface | Os alertas relacionados à oxigenação, perda de sinal ou mau posicionamento do sensor devem ser compreensíveis para o terapeuta durante a sessão. |
| RNF12 | Rastreabilidade | Os dados fisiológicos registrados pelo módulo Spo2 devem estar associados à sessão, ao paciente e ao momento em que foram coletados. |
| RNF13 | Integridade dos dados | Os dados de oxigenação, frequência cardíaca e eventos de alerta devem ser armazenados sem sobrescrever ou corromper os dados já registrados pelo software I Blue It. |
| RNF14 | Persistência | Os registros fisiológicos da sessão devem permanecer disponíveis para consulta após o encerramento do jogo. |
| RNF15 | Privacidade | Os dados fisiológicos do paciente devem ser tratados como informações sensíveis e não devem ser expostos indevidamente. |
| RNF16 | Manutenibilidade | O código do módulo Spo2 deve ser organizado de forma a facilitar ajustes, correções e futuras expansões. |
| RNF17 | Testabilidade | O módulo Spo2 deve permitir testes com dados simulados, possibilitando verificar conexão, validação de sinal, alertas e registro de dados sem depender exclusivamente do sensor físico. |
| RNF18 | Extensibilidade | A estrutura do módulo Spo2 deve tem em mente a modularidade com o fim de permitir futura adaptação para outros sensores fisiológicos, caso necessário. |
| RNF19 | Consistência | As nomenclaturas, unidades e formas de apresentação dos dados fisiológicos devem ser consistentes em todo o sistema. |
| RNF20 | Recuperação de falhas | O módulo Spo2 deve permitir a retomada do monitoramento após falha temporária de conexão ou reposicionamento do sensor. |
| RNF21 | Compatibilidade | O módulo Spo2 deve ser codificado em linguagens e tecnologias já presentes no software |

---

## 2.5 Regras de Negócio

As regras de negócios aqui presentes foram elencadas com a visão apenas da melhoria, tratando tanto o modulo da melhoria(módulo Spo2), quanto o ecossitema I Blue It já existente, apenas como um unico sistema unificado, porém abrangendo apenas os apectos presentes no I Blue IT que compentem a ser tratados na melhoria.Assim não re-retiramos as regras de negócios nos ecosistema já presente.

- Apenas o terapeuta pode definir o limite mínimo de oxigenação.
- O terapeuta responsável pode pausar ou parar a sessão a qualquer momento.
- O terapeuta responsável pode adaptar a sessão comforme sua análise clínica.
- Em situações fisiológicas críticas, a segurança do paciente deve prevalecer em relação a continuidade da sessão em andamento.
- Uma sessão fisioterapica apenas podem ocorrer se os equipamentos de medição estiverem aptos, posicionados e captando os sinais vitais do paciente corretamente.
- Durante uma sessão caso ocorra problemas de medições devido a problemas no equipamento, mal posicionamento e ou no processamento dos dados o sistema deve pausar a sessão, e notificar o usuario (paciente e medico), 
- Durante as sessões o sistema deve avisar o medico de anomalias nos biosinais.
- Em casos de parada da sessão o sistema deve manter os dados já obtidos da sessão, e possibilitar tanto a retomada do exercicio do estado atual do exercicio, ou eventual encerramento da sessão.  
- Apenas a equipe médica e o paciente, devem ter acesso aos dados fisiologicos do paciente.
- O sistema deve disponibilizar os dados captadas do paciente há equipe medica.
- A sessão pausada só pode ser retomada após autorização do terapeuta.
- A sessão deve ser pausada caso o sistema identifique risco fisiológico ao paciente.
- A leitura do modulo Spo2 só deve ser considerada válida se houver contato adequado do sensor com o paciente.
- A oxigenação deve ser calculada somente com dados válidos.
- Em caso de leituras inválidas ou instáveis, não devem ser utilizadas tais medições para o calculo da oxigenação do paciente.
- O limite mínimo de oxigenação definido pelo terapeuta deve ser usado como referência para classificar a condição fisiológica do paciente durante a sessão.
- O limite mínimo de oxigenação estipulado pelo terapeuta deve ser usado como referência para analisar a condição fisiológica do paciente durante a sessão.
- Os dados do novo modulo Sp02 não devem substituir os dados já presentes no ecosistema I Blue It.
- Os dados do novo modulo Sp02 devem ser integrados aos dados já presentes no ecosistema I Blue It.
- Eventos de alerta tais como queda de oxigenação, perda de sinal e ou pausa da sessão devem ser registrados no histórico da sessão.
- A frequência cardíaca deve ser registrada sempre que possivel pelo modulo Sp02.
- Em casos de muito breve ausência de sinal fisiológico não devem ser interpretadas automaticamente como condições crítica do paciente.
- O terapeuta responsável deve ter acesso aos registros fisiológicos da sessão após seu encerramento.

---

## 2.6 Fora do Escopo

Aqui constam alguns pontos que está proposta não realizará, apenas ilustrados com fim de sanar eventuais duvidas sobre o escopo do projeto(Entenda o sistema como a melhoria a ser desenvolvida).

- O sistema não realizará diagnóstico médico do paciente.
- O sistema não prescreverá sessões de exercícios respiratórios de forma autônoma.
- O sistema não alterará a lógica principal de jogabilidade e utilização já presentes no I Blue It.
- O sistema não realizará integração com eventuais prontuários eletrônicos externos.
- O sistema não realizará tomada de decisão clínica autonoma via inteligência artificial .
- O sistema não adicionará sensores fisiológicos adicionais além do sensor de Spo2.

---

# 3. Fluxos e Comportamento do Sistema

Para descrever o fluxo do software nestá secção, utiliza-se o diagrama de sequência, no qual está contido as principais funções do sistema I Blue It já com aprimoramento proposto.

---

## 3.1 Fluxo Principal do Usuário

Abaixo pode-se observar o diagrama de sequência na integra, ao qual contém o fluxo principal do sistema: 

<img width="5784" height="3804" alt="image" src="https://github.com/user-attachments/assets/a364c9cb-7605-45e5-90ce-2392111606e4" />


O fluxo pode ser subdivido em três etapas distintas de operação, sendo elas:
- Primeira etapa é feita da preparação de sessão fisioterápica, nesta etapa o sistema é configurado pelo fisioterapeuta para se adequar ao paciente, além da checagem se o própio sistema está operando corretamento.

<img width="5812" height="2212" alt="image" src="https://github.com/user-attachments/assets/b13b23e1-24f4-43be-947e-1a3256dd25b4" />

- Segunda etapa consiste na execução da sessão fisioterapica em si, no qual o paciente realiza o exercicio superfisionado pelo fisioterapeuta, e no qual o sistema capta todos dados para analise posterior.

<img width="5452" height="2124" alt="image" src="https://github.com/user-attachments/assets/f6bd5a6f-eca8-4b3c-a410-6e886cada184" />

- Terceira etapa, nesta etapa o sistema organiza,armazena e disponibiliza para o terapeuta todos os dados obtidos pela sessão, e se necessario de sessões anteriores. 

<img width="5452" height="3632" alt="image" src="https://github.com/user-attachments/assets/dab5a250-47fb-4bcd-a7f6-6dc7aa5ef82a" />


Apresente o fluxo principal do sistema.

Utilize:

- fluxogramas
- diagramas de atividades
- diagramas de sequência

Inclua **imagens dos diagramas**.

---

## 3.2 Fluxos Alternativos

erros de conexão posicionamento, ou interrupções

Descreva cenários como:

- erros
- cancelamentos
- exceções

---

# 4. Mockups e Experiência do Usuário (UX)

Esta seção apresenta **a visualização inicial do produto antes da implementação**.

Mockups ajudam a validar:

- fluxo de navegação
- organização da interface
- interações do usuário
- clareza da experiência

Ferramentas sugeridas:

- Figma
- Excalidraw
- Balsamiq
- Whimsical
- protótipos desenhados à mão

---

## 4.1 Fluxo de Navegação

Apresente um diagrama mostrando como o usuário navega entre telas.

Exemplo:

Login → Dashboard → Cadastro → Relatório

Inclua **imagem do fluxo de navegação**.

---

## 4.2 Wireframes ou Mockups das Telas

Apresente os principais mockups do sistema.

Inclua pelo menos:

- tela inicial
- fluxo principal
- tela de entrada de dados
- tela de resultado ou visualização

Para cada tela inclua:

- imagem
- breve descrição da funcionalidade
- ações principais do usuário

Sempre que possível:

- inclua **links para protótipo navegável**
- inclua **prints das telas**

---

## 4.3 Fluxo de Interação do Usuário

Demonstre passo a passo um fluxo importante.

Exemplo:

1. usuário acessa o sistema  
2. cria conta  
3. registra dados  
4. visualiza resultados  

Inclua **sequência de telas ou fluxo visual**.

---

## 4.4 Feedback Inicial de Usuários (Opcional)

Se possível, inclua:

- comentários de usuários
- sugestões de melhoria
- validação inicial do mockup

---

# 5. Arquitetura do Sistema

Esta seção demonstra **como o sistema será construído**.

---

## 5.1 Diagrama C4

Apresente três níveis.
## 1. Nível 1: Diagrama de Contexto
É a **visão macro** do sistema. O foco aqui não é a tecnologia, mas sim como o software se encaixa no ecossistema e no mundo real.

* **Objetivo:** Mostrar o sistema como uma "caixa preta" e suas interações básicas com o ambiente externo.
* **O que incluir:**
    * **Atores:** Diferentes perfis de usuários (Ex: Cliente, Administrador, Operador).
    * **Sistemas Externos:** Softwares legados, serviços de terceiros ou provedores de identidade.
    * **Fluxo de Valor:** Como a informação entra, circula e sai do sistema principal.

---

## 2. Nível 2: Diagrama de Containers
Neste estágio, damos o primeiro **"zoom"**. Decompomos o sistema em suas unidades de execução independentes (containers).

* **Objetivo:** Apresentar a arquitetura de alto nível e as decisões tecnológicas fundamentais.
* **O que incluir:**
    * **Aplicações Web/Mobile:** Interfaces de usuário (Ex: SPA em React, App Android/iOS).
    * **Serviços de Backend:** Unidades lógicas de processamento (Ex: API Gateway, Microserviços em Node.js ou Go).
    * **Armazenamento:** Persistência de dados (Ex: PostgreSQL, MongoDB, Redis).
    * **Protocolos:** Como os containers se comunicam (Ex: JSON/HTTPS, gRPC, RabbitMQ).

---

## 3. Nível 3: Diagrama de Componentes
O foco agora é o que acontece **dentro de um único container** (como uma API específica ou um serviço de backend).

* **Objetivo:** Identificar as responsabilidades internas, padrões de código e a organização lógica.
* **O que incluir:**
    * **Estrutura Interna:** Organização das camadas (Ex: Controladores, Serviços, Repositórios e Clientes de API).
    * **Lógica de Negócio:** Componentes que encapsulam as regras específicas do domínio.
    * **Interações:** Como os componentes internos se orquestram para processar e responder a uma requisição.
---

## 5.2 Modelo de Dados

Apresente:

- DER (diagrama entidade relacionamento)
- esquema relacional
- modelo de documentos (NoSQL)

Inclua **diagramas do modelo de dados**.

---

## 5.3 Principais Componentes

Descreva os principais módulos do sistema.

Exemplo:

- API
- sistema de autenticação
- módulo de processamento
- camada de persistência

---

## 5.4 Stack Tecnológica

Liste as tecnologias utilizadas.

Para cada tecnologia explique **por que ela foi escolhida**.

Exemplo:

Node.js  
Escolhido pela capacidade de lidar com alto volume de requisições I/O.

---

# 6. Segurança e Privacidade

Inclua preocupações básicas de segurança.

Exemplos:

- proteção contra OWASP Top 10
- autenticação e autorização
- criptografia de dados sensíveis

---

## 6.1 Privacidade e LGPD

Explique:

- quais dados serão coletados
- como serão armazenados
- como o usuário poderá solicitar remoção de dados

---

# 7. Planejamento do Projeto

Defina os principais marcos de desenvolvimento.

| Marco | Descrição | Prazo |
|---|---|---|
| M1 | Setup do ambiente e prova de conceito | Semana X |
| M2 | MVP funcional | Semana Y |
| M3 | Testes e melhorias | Semana Z |

---

# 8. Referências

Inclua:

- artigos
- documentação técnica
- ferramentas utilizadas
- repositórios

---

# 9. Apêndices

Podem incluir:

- mockups adicionais
- resultados de pesquisa
- entrevistas com usuários
- diagramas complementares
- links para protótipos ou repositórios

Sempre que possível inclua **imagens, protótipos ou referências visuais**.

---

# 10. Parecer do Comitê de Avaliação

(A ser preenchido pelos professores)

**Avaliador 1:** __________________________  
**Status:** [ ] Aprovado  [ ] Ajustar

Observações:

---

**Avaliador 2:** __________________________  
**Status:** [ ] Aprovado  [ ] Ajustar

Observações:

---

**Avaliador 3:** __________________________  
**Status:** [ ] Aprovado  [ ] Ajustar

Observações:

