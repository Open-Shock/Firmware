// automatically generated by the FlatBuffers compiler, do not modify

import * as flatbuffers from 'flatbuffers';

export class OtaUpdateSetUpdateChannelCommand {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):OtaUpdateSetUpdateChannelCommand {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsOtaUpdateSetUpdateChannelCommand(bb:flatbuffers.ByteBuffer, obj?:OtaUpdateSetUpdateChannelCommand):OtaUpdateSetUpdateChannelCommand {
  return (obj || new OtaUpdateSetUpdateChannelCommand()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsOtaUpdateSetUpdateChannelCommand(bb:flatbuffers.ByteBuffer, obj?:OtaUpdateSetUpdateChannelCommand):OtaUpdateSetUpdateChannelCommand {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new OtaUpdateSetUpdateChannelCommand()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

channel():string|null
channel(optionalEncoding:flatbuffers.Encoding):string|Uint8Array|null
channel(optionalEncoding?:any):string|Uint8Array|null {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? this.bb!.__string(this.bb_pos + offset, optionalEncoding) : null;
}

static startOtaUpdateSetUpdateChannelCommand(builder:flatbuffers.Builder) {
  builder.startObject(1);
}

static addChannel(builder:flatbuffers.Builder, channelOffset:flatbuffers.Offset) {
  builder.addFieldOffset(0, channelOffset, 0);
}

static endOtaUpdateSetUpdateChannelCommand(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createOtaUpdateSetUpdateChannelCommand(builder:flatbuffers.Builder, channelOffset:flatbuffers.Offset):flatbuffers.Offset {
  OtaUpdateSetUpdateChannelCommand.startOtaUpdateSetUpdateChannelCommand(builder);
  OtaUpdateSetUpdateChannelCommand.addChannel(builder, channelOffset);
  return OtaUpdateSetUpdateChannelCommand.endOtaUpdateSetUpdateChannelCommand(builder);
}
}
