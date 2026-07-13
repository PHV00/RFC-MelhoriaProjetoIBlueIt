arquivo.h → declaração, interface, tipos disponíveis
arquivo.c → implementação, código que realmente executa

.h interface com os metodos apenas em formato de interface

.c
codigo em si

rodar modo de desenvolvedor para passar o esp32-c3

. $HOME/esp/esp-idf/export.sh

fluxo dos dados:

1. Coletar amostra bruta
2. Validar erro de comunicação
3. Adicionar timestamp e sequência
4. Inserir no buffer circular
5. Verificar se há amostras suficientes
6. Avaliar qualidade da janela
7. Detectar presença do dedo/contato
8. Calcular AC e DC
9. Calcular frequência cardíaca
10. Calcular razão R e estimativa de SpO₂
11. Calcular confiança final
12. Publicar health_frame_t

 