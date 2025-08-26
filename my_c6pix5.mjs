import * as m from 'zigbee-herdsman-converters/lib/modernExtend';

export default {
  zigbeeModel: ['DIY-C6-PIX5'],          // muss exakt zum Basic "model" passen
  model: 'DIY-C6-PIX5',
  vendor: 'Espressif',
  description: 'ESP32-C6 NeoPixel – 5x Color-dimmable, 5 endpoints (10..14)',

  // BENANNTE Endpoints -> saubere Suffixe in HA
  endpoint: (device) => ({
    ep10: 10,
    ep11: 11,
    ep12: 12,
    ep13: 13,
    ep14: 14,
  }),
  meta: { multiEndpoint: true },

  // Ein einziges light-Extend für ALLE Endpoints:
  extend: [
    m.light({
      endpointNames: ['ep10','ep11','ep12','ep13','ep14'],
      brightness: true,
      color: { modes: ['xy','hs'] },   // xy + Hue/Sat
    }),
  ],
};