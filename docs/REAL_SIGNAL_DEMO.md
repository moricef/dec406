# Reproducible real-signal demonstration

This demonstration validates the complete SGB receive chain on a published
over-the-air IQ recording. It does not inject a hexadecimal frame directly:
`dec406_iq` performs carrier acquisition, DSSS/OQPSK despreading, BCH
validation and T.018 field decoding from complex samples.

The recording was made at the firmin relay station, about 80 km from Toulouse,
with a Nooelec NESDR SMArTee v5 and an omnidirectional antenna. It is distributed
as SigMF `ci16_le`, sampled at 2.4576 Msps and centred on 406.050 MHz.

Run from the repository root:

```bash
make demo-real-sgb
```

The command downloads the published sample archive, verifies its SHA-256,
builds `dec406_iq`, and decodes the normal-mode capture. The expected result
includes:

```text
BCH validated: boxcar offset 16/64 Costas phase 0deg nerr=0
Frame Mode: Normal
23 Hex ID: 9C77FFFE1A7F00000000000
Country Code: 227 (France)
Type: System Beacon
Position: 43.56049 N, 1.48080 E
```

Runtime depends on the CPU. The temporary download and extracted files are
removed when the demonstration exits.

