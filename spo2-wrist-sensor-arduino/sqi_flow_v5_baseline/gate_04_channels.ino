/*
 * GATE 04 - COERENCIA ENTRE CANAIS RED <-> IR
 * ============================================
 *
 * STATUS: DOCUMENTADO, AINDA NAO IMPLEMENTADO.
 * O codigo deste Gate so sera escrito depois de G1-G3 serem validados.
 *
 * PERGUNTA DO GATE
 * ----------------
 * "RED e IR estao ambos utilizaveis e descrevem uma historia temporal
 *  compativel para que a janela possa seguir para a oximetria?"
 *
 * FUNDAMENTACAO
 * -------------
 * Desquins et al. - A Survey of Photoplethysmography and Imaging
 * Photoplethysmography Quality Assessment Methods.
 * Ano: 2022
 * DOI: 10.3390/app12199582
 *
 * A survey fundamenta a avaliacao de qualidade por multiplas caracteristicas
 * e a necessidade de avaliar a adequacao do sinal para a aplicacao.
 *
 * ORIGEM DESTE GATE
 * -----------------
 * G4 e uma ADAPTACAO DE ARQUITETURA DO PROJETO para o caso de oximetria
 * dual-channel do MAX30102. Nao afirmamos que Desquins et al. propoem este
 * Gate exatamente nesta forma, nem existe na fonte selecionada um threshold
 * universal RED<->IR que possamos copiar.
 *
 * O QUE VAMOS VALIDAR
 * -------------------
 *
 *   RED valido ---------\
 *                        +--> periodos / ACF compativeis? --> SIM --> PPG VALID
 *   IR valido ----------/                                |
 *                                                         +--> NAO --> INVALID
 *
 * Parametros previstos:
 * - disponibilidade dos dois canais;
 * - periodo/lag dominante por canal;
 * - compatibilidade das metricas de periodicidade;
 * - tolerancia RED/IR a ser caracterizada no MAX30102.
 */

// TODO: implementar depois da validacao dos Gates 01, 02 e 03.
