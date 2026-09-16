# Auditoria evolutiva do firmware de oximetria

## Escopo

Esta revisão descreve o código de `spo2-wrist-sensor/main` no commit `a74986d93098367b9e0b2085ed6d113f29ffa1d0`, por inspeção estática realizada em 2026-07-15.

Não foram executados build, hardware, replay ou comparação com referência. Assim:

- “implementado” significa que existe um caminho de código integrado;
- não significa que o caminho compilou ou funcionou no hardware;
- não significa que HR/SpO₂ sejam exatas;
- não significa segurança ou eficácia clínica.

As pendências deste documento são estados do baseline e backlog de evolução. Quando forem corrigidas e testadas, o documento deve ser atualizado; elas não são limitações arquiteturais permanentes.

## 1. Resumo da evolução

A análise anterior deste arquivo descrevia uma versão em que HR e SpO₂ eram stubs e apenas RAW chegava ao controlador. Isso não corresponde ao HEAD atual. O Git preserva a análise histórica; esta versão passa a representar o baseline corrente.

| Área | Estado estático atual | Evolução observada | Próxima comprovação necessária |
|---|---|---|---|
| identificação do sensor | parcial | PART_ID/REV_ID são lidos e emitidos | seleção explícita MAX30102 + teste HIL |
| configuração | parcial | reset, escrita e readback foram implementados | versionamento completo e teste de registradores |
| FIFO | parcial | ponteiros/overflow são consultados e a FIFO é drenada em lote | TST-001 com taxa, overflow e recuperação |
| timestamps | parcial | amostras do lote recebem tempo pela taxa nominal | TST-002, gaps, jitter e taxa efetiva |
| buffer | implementado estaticamente | sobrescritas são contadas | teste unitário e política de perda |
| qualidade | protótipo parcial | gate multicomponente substituiu máximo-mínimo simples | dataset rotulado, falso válido e calibração dos limiares |
| HR | protótipo parcial | há detecção de picos, IBI e estabilidade | replay e comparação com referência |
| SpO₂ | protótipo não calibrado | razão dos quocientes e curva configurável foram implementadas | curva do conjunto final e TST-005 |
| confiança | parcial | combina qualidade, HR e SpO₂; separa validade clínica | validar score e completar contrato externo |
| estados/recuperação | parcial | transições são validadas e há tentativa de recuperação | testes de falhas, watchdog e recuperação segura |
| telemetria | contraditória com o contrato desejado | frame JSON versionado existe | incluir validade clínica, calibração, motivos, overflow, IDs e RAW |
| persistência | ausente | `config_repo` ainda é constante compilada | definir NVS/armazenamento, migração e rastreabilidade |
| testes | ausentes no repositório | protocolo e IDs foram especificados | implementar unidade, replay, integração e HIL |
| integração I Blue It | não verificável neste repositório | arquitetura e requisitos estão documentados | commits dos demais sistemas e teste ponta a ponta |

## 2. Aquisição e driver

### 2.1 O que existe

`max3010x_driver.c` atualmente:

- lê `REV_ID` e `PART_ID`;
- aguarda o bit de reset limpar com timeout;
- limpa a FIFO;
- configura taxa, faixa ADC, largura de pulso, correntes e modo;
- verifica registradores por readback;
- consulta ponteiros e contador de overflow;
- lê todas as amostras disponíveis, limitado à profundidade da FIFO;
- conta eventos de overflow.

`i2c_bus.c` valida estado/argumentos, usa timeout de 50 ms e protege acesso com mutex.

Essas mudanças tratam vários problemas da análise histórica, mas ainda precisam de teste automatizado e hardware para serem consideradas confirmadas operacionalmente.

### 2.2 Pendências evolutivas

1. **Variante:** `max3010x_probe` aceita qualquer PART_ID que não seja `0x00` ou `0xFF`. Falta um perfil explícito de MAX30102, capacidades e rejeição de variante incompatível.
2. **Configuração completa:** a configuração ainda não carrega todos os parâmetros por perfil/versionamento; FIFO averaging, ADC e pulse width são codificados no driver.
3. **Data ready:** interrupções estão desabilitadas; a aquisição depende de polling.
4. **Período divergente:** `config_repo` declara polling de 10 ms, mas `main.c` usa atraso fixo de 50 ms.
5. **Gaps:** a sequência reflete amostras inseridas no buffer, não prova continuidade de todas as amostras produzidas pelo sensor.
6. **Tempo:** timestamps são reconstruídos pela taxa nominal a partir do instante de leitura do lote; ainda falta validar atraso, jitter e taxa efetiva.
7. **Overflow:** a janela é limpa quando um overflow é detectado, mas o comportamento completo precisa de oráculo e HIL.
8. **Shutdown:** ainda não existe caminho explícito de desligamento seguro dos LEDs em todos os erros.

## 3. Buffer e janela

O buffer circular de 512 amostras mantém ordem cronológica, conta sobrescritas e permite copiar a janela mais recente. Em 100 Hz, comporta aproximadamente 5,12 s; a configuração usa 400 amostras, aproximadamente 4 s.

Pendências:

- teste de wrap-around e ordenação;
- tornar perda/sobrescrita um evento observável no frame;
- snapshot/sincronização antes de introduzir múltiplas tarefas;
- associar cada janela a configuração, sequência inicial/final e gaps.

## 4. Qualidade do sinal

O código atual calcula:

- remoção linear de tendência;
- DC de RED/IR;
- AC RMS;
- ruído residual após média móvel;
- SNR;
- índice de perfusão;
- correlação RED/IR;
- continuidade temporal;
- fração de clipping;
- escore e bitmask de motivos de invalidade.

Isso é um avanço em relação à heurística histórica. Contudo, `MIN_DC_IR`, `MIN_AC_RMS`, perfusão, correlação, clipping, pesos e faixas do score ainda são hipóteses de engenharia. Devem ser avaliados com replay rotulado e erro contra referência antes de serem usados para alegar confiança calibrada.

Movimento continua sem sensor inercial ou modelo específico. A evolução pode combinar rejeição conservadora, acelerômetro ou compensação, desde que testada por condição.

## 5. Frequência cardíaca

`hr_estimator_compute` está implementado e integrado. Ele usa:

1. remoção de tendência no IR;
2. média móvel de cinco pontos;
3. limiar proporcional ao desvio RMS;
4. picos positivos e negativos;
5. distância mínima pela faixa de BPM;
6. mediana dos intervalos válidos;
7. coeficiente de variação para confiança.

Pendências:

- replay com sinais conhecidos;
- teste de pico duplo, metade da frequência e arritmia;
- sensibilidade a movimento e baixa perfusão;
- comparação simultânea com referência;
- definição do domínio apropriado à população/intended use.

## 6. SpO₂

`spo2_estimator_compute_with_calibration` calcula:

```text
R = (AC_RED / DC_RED) / (AC_IR / DC_IR)
SpO₂ = aR² + bR + c
```

O código verifica domínio, finitude e faixa numérica. A configuração atual usa `SpO₂ = 110 - 25R`, `calibrated=false`, `version=0` e permite estimativa não calibrada.

Interpretação correta: existe uma estimativa experimental para testar o pipeline. Ela ainda não possui curva rastreável para o conjunto óptico final e não deve controlar ação clínica.

Próximas evoluções:

- registrar hardware, encapsulamento e configuração óptica;
- coletar pares simultâneos com referência;
- aprovar critérios antes da coleta;
- ajustar curva e domínio sem apagar resultados negativos;
- versionar coeficientes e associá-los a cada frame;
- medir MAE, viés e limites de concordância.

## 7. Confiança e validade

O modelo interno possui:

- qualidade e motivos de invalidade;
- validade/status/confiança de HR;
- validade/status/confiança/calibração de SpO₂;
- `health_frame.valid`;
- `health_frame.clinical_valid`;
- contador de overflow.

Limitação atual crítica: uma SpO₂ não calibrada pode ser `spo2.valid=true` e contribuir para `health_frame.valid=true`, enquanto `clinical_valid=false`. Essa distinção é aceitável internamente para uma POC somente se o consumidor também a receber e respeitar.

O score de confiança continua heurístico. Ele deve ser validado por curvas de confiabilidade e erro observado, não apenas por plausibilidade do valor.

## 8. Telemetria

`sensor_info` transmite versão do protocolo, PART_ID, REV_ID e taxa. O frame ativo transmite estado, timestamp, `valid`, presença de dedo, qualidade, perfusão, HR e SpO₂.

A versão detalhada do frame está comentada. Consequentemente, o contrato ativo não inclui:

- `clinical_valid`;
- motivos de invalidade;
- overflow;
- SNR, correlação, continuidade e clipping;
- status dos estimadores;
- calibração e versão;
- firmware/configuração;
- paciente/sessão/dispositivo/sequência.

Além disso, `serial_telemetry_print_sample` não imprime RED/IR porque o código está comentado.

Prioridade: definir um frame versionado completo e um teste de contrato antes da integração com Unity ou mecanismos de segurança. Valores inválidos devem continuar explicitamente desconhecidos no consumidor, mesmo que campos numéricos estejam presentes por compatibilidade.

## 9. Máquina de estados e recuperação

As transições entre BOOT, SELF_TEST, IDLE, SAMPLING, TRACKING, LOW_CONFIDENCE e ERROR agora são validadas. Após três erros de aquisição, o sistema entra em ERROR e tenta reconfiguração a cada segundo.

Ainda faltam:

- eventos e razões estruturadas de transição;
- teste de todas as transições;
- distinção entre erro transitório e permanente;
- limite/política de tentativas;
- desligamento seguro;
- watchdog para amostra repetida/congelada;
- notificação que impeça uma sessão externa de continuar silenciosamente.

## 10. Configuração e persistência

`config_repo` é uma estrutura constante no binário. Não há NVS, schema, migração, rollback ou persistência de calibração. Parte da configuração, como pulse width e ADC, permanece dentro do driver.

Evolução recomendada:

- reunir parâmetros do sensor em perfil versionado;
- separar configuração técnica de política clínica;
- validar faixas e compatibilidade por variante;
- registrar hash/versão no frame e no dataset;
- definir persistência somente depois de requisitos de integridade e rollback.

## 11. Segurança clínica

A pasta `safety` contém avaliação de confiança; não contém uma política clínica de segurança. Não existem no firmware atual persistência de queda, histerese, regras 95/89, pausa de jogo, override ou integração com terapeuta.

Essa ausência impede ação clínica prematura, mas também significa que requisitos de segurança ainda não foram implementados. A próxima etapa deve continuar usando replay/simulação até que medição, política e operador sejam aprovados.

## 12. Testes e evidência

Não foram localizados testes automatizados ou HIL versionados. Também não foram localizados resultados de MET-001 a MET-010 para este commit.

Ordem mínima sugerida:

1. unidade: registradores, encoding, readback e buffer;
2. FIFO/timestamps: taxa, lote, overflow, gaps e recuperação;
3. replay: qualidade, HR, SpO₂, validade e determinismo;
4. falhas: I²C, dedo ausente, clipping, movimento e sinal congelado;
5. HIL: configuração e temporização física;
6. referência: HR/SpO₂ pareadas;
7. integração: frame, persistência, jogo e dashboard;
8. segurança: política, override e alertas.

Teste de software não demonstra eficácia clínica.

## 13. Prioridades atuais

1. completar identidade/configuração do MAX30102 e comprovar FIFO/timestamps;
2. preservar RAW e implementar replay/testes;
3. corrigir o contrato de telemetria e validade;
4. validar o gate de qualidade e HR;
5. calibrar e comparar SpO₂;
6. integrar sistemas externos com rastreabilidade;
7. somente então testar políticas de segurança e DeepDDA.

## 14. Formulação segura do estado atual

> O repositório contém uma prova de conceito em código para aquisição PPG, avaliação de qualidade, estimativa de frequência cardíaca e estimativa não calibrada de SpO₂. O pipeline ainda requer testes automatizados e HIL, calibração, comparação com referência, telemetria completa e integração rastreável. Não há evidência de validade clínica ou eficácia.

Essa formulação deve evoluir quando novos testes e resultados forem versionados; não deve ser repetida como conclusão sobre versões futuras.
