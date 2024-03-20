+------------------------------------------------------------------------------+
| I2S ESSENTIALS                                                               |
+------------------------------------------------------------------------------+

*-----------------------------*            *----------*
|                   clock SCK |------------|          |
| Transmitter  word select WS |------------| Recevier |
|              serial data SD |------------|          |
*-----------------------------*            *----------*

@Serial data is transmitted in two’s complement with the MSB first.

@The word select line indicates the channel being transmitted:
WS = 0 => channel 1 (left)
WS = 1 => channel 2 (right)

The bus has only to handle audio data, while the other signals, such as
sub-coding and control, are transferred separately.

--------------------------------------------------------------------------------
