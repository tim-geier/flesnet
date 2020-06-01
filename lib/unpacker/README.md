# Unpacker prototype in tsclient
## Existing options
(incomplete; just unpacker related options)

| Option                         | Alt | Description                                                          |
|--------------------------------|-----|----------------------------------------------------------------------|
| --input-archive                | -i  | Set input .tsa file(s)                                               |
| --multi-input                  | -m  | Enable multi archive/stream input                                    |
| --maximum-number               | -n  | Set the maximum number of timeslices to process (default: unlimited) |


To process more than one timeslice-archive use `tsclient -m -i "filename" ...`  
With multi-input enabled the filename parameter will be parsed differently:
| Parameters           | Opened files                    |
|----------------------|---------------------------------|
| `-m -i "file1;file2"`| `file1` and `file2`             |
| `-m -i "file[1-3]"`  | `file1`,`file2` and `file3`     |
| `-m -i "file*"`      | All files beginning with `file` |




## New options added to tsclient

| Option                         | Alt | Description                                              |
|--------------------------------|-----|----------------------------------------------------------|
| --unpack                       | -u  | Enable unpacking                                         |
| --tof-unpacker-mapping         |     | Set mapping-file; (default: ./mapping.par)               |
| --tof-unpacker-output-filename |     | Set output filename; (default: \<input-filename\>.digi)  |
| --tof-unpacker-merge-output    |     | Merge output in one big .digi file                       |

If none of the following options is present, all supported detectors will be processed. Otherwise just the selected ones.
| Option            | Description                          |
|-------------------|--------------------------------------|
| --unpack-tof      | Enable TOF unpacking (SYS-ID 0x60)   |
| --unpack-t0       | Enable T0 unpacking (SYS-ID 0x90)    |

## Known problems

 - Overflowing epoch within microslice should increase epoch-cycle -- not implemented yet
 - Data with epoch = 0 will be ignored if no following epoch is found or TOF_UNPACKER_USE_MISSING_EPOCH_QUIRKS_MODE is disabled. Maybe use 0xffffffff (valid epoch can be 31 bits max) for unknown epoch
 