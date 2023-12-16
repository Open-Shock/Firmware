// automatically generated by the FlatBuffers compiler, do not modify

import * as flatbuffers from 'flatbuffers';

import { DeviceToLocalMessagePayload, unionToDeviceToLocalMessagePayload, unionListToDeviceToLocalMessagePayload } from '../../../open-shock/serialization/local/device-to-local-message-payload';


export class DeviceToLocalMessage {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):DeviceToLocalMessage {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsDeviceToLocalMessage(bb:flatbuffers.ByteBuffer, obj?:DeviceToLocalMessage):DeviceToLocalMessage {
  return (obj || new DeviceToLocalMessage()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsDeviceToLocalMessage(bb:flatbuffers.ByteBuffer, obj?:DeviceToLocalMessage):DeviceToLocalMessage {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new DeviceToLocalMessage()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

payloadType():DeviceToLocalMessagePayload {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? this.bb!.readUint8(this.bb_pos + offset) : DeviceToLocalMessagePayload.NONE;
}

payload<T extends flatbuffers.Table>(obj:any):any|null {
  const offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? this.bb!.__union(obj, this.bb_pos + offset) : null;
}

static startDeviceToLocalMessage(builder:flatbuffers.Builder) {
  builder.startObject(2);
}

static addPayloadType(builder:flatbuffers.Builder, payloadType:DeviceToLocalMessagePayload) {
  builder.addFieldInt8(0, payloadType, DeviceToLocalMessagePayload.NONE);
}

static addPayload(builder:flatbuffers.Builder, payloadOffset:flatbuffers.Offset) {
  builder.addFieldOffset(1, payloadOffset, 0);
}

static endDeviceToLocalMessage(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static finishDeviceToLocalMessageBuffer(builder:flatbuffers.Builder, offset:flatbuffers.Offset) {
  builder.finish(offset);
}

static finishSizePrefixedDeviceToLocalMessageBuffer(builder:flatbuffers.Builder, offset:flatbuffers.Offset) {
  builder.finish(offset, undefined, true);
}

static createDeviceToLocalMessage(builder:flatbuffers.Builder, payloadType:DeviceToLocalMessagePayload, payloadOffset:flatbuffers.Offset):flatbuffers.Offset {
  DeviceToLocalMessage.startDeviceToLocalMessage(builder);
  DeviceToLocalMessage.addPayloadType(builder, payloadType);
  DeviceToLocalMessage.addPayload(builder, payloadOffset);
  return DeviceToLocalMessage.endDeviceToLocalMessage(builder);
}
}
