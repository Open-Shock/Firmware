// automatically generated by the FlatBuffers compiler, do not modify

import * as flatbuffers from 'flatbuffers';

export class SetRfTxPinCommand {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):SetRfTxPinCommand {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

pin():number {
  return this.bb!.readUint8(this.bb_pos);
}

static sizeOf():number {
  return 1;
}

static createSetRfTxPinCommand(builder:flatbuffers.Builder, pin: number):flatbuffers.Offset {
  builder.prep(1, 1);
  builder.writeInt8(pin);
  return builder.offset();
}

}
