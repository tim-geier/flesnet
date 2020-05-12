# Unpacker prototype in tsclient

## Options added to tsclient

| Option                         | Alt | Description                                          |
|--------------------------------|-----|------------------------------------------------------|
| --unpack                       | -u  | Enable unpacking                                     |
| --tof-unpacker-mapping         |     | Set mapping-file; defaults to ./mapping.par          | 
| --tof-unpacker-output-filename |     | Set output filename; defaults to input-filename.digi |
| --multi-input                  | -m  | Must be used in combination with --input-archive     |
| --input-archive                | -i  | Set input .tsa file(s)                               |

At least one detector must be selected to run the unpacker:
| Option            | Description                                          |
|-------------------|------------------------------------------------------|
| --unpack-tof      | Enable TOF unpacking (SYS-ID 0x60)                   |
| --unpack-t0       | Enable T0 unpacking (SYS-ID 0x90)                    |
