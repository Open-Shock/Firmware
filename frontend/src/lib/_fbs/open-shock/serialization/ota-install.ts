// automatically generated by the FlatBuffers compiler, do not modify

import * as flatbuffers from 'flatbuffers';

import { SemVer } from '../../open-shock/sem-ver';


export class OtaInstall {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):OtaInstall {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsOtaInstall(bb:flatbuffers.ByteBuffer, obj?:OtaInstall):OtaInstall {
  return (obj || new OtaInstall()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsOtaInstall(bb:flatbuffers.ByteBuffer, obj?:OtaInstall):OtaInstall {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new OtaInstall()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

version(obj?:SemVer):SemVer|null {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? (obj || new SemVer()).__init(this.bb!.__indirect(this.bb_pos + offset), this.bb!) : null;
}

static startOtaInstall(builder:flatbuffers.Builder) {
  builder.startObject(1);
}

static addVersion(builder:flatbuffers.Builder, versionOffset:flatbuffers.Offset) {
  builder.addFieldOffset(0, versionOffset, 0);
}

static endOtaInstall(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createOtaInstall(builder:flatbuffers.Builder, versionOffset:flatbuffers.Offset):flatbuffers.Offset {
  OtaInstall.startOtaInstall(builder);
  OtaInstall.addVersion(builder, versionOffset);
  return OtaInstall.endOtaInstall(builder);
}
}
