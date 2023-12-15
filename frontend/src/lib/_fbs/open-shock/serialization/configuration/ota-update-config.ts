// automatically generated by the FlatBuffers compiler, do not modify

import * as flatbuffers from 'flatbuffers';

export class OtaUpdateConfig {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):OtaUpdateConfig {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsOtaUpdateConfig(bb:flatbuffers.ByteBuffer, obj?:OtaUpdateConfig):OtaUpdateConfig {
  return (obj || new OtaUpdateConfig()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsOtaUpdateConfig(bb:flatbuffers.ByteBuffer, obj?:OtaUpdateConfig):OtaUpdateConfig {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new OtaUpdateConfig()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

/**
 * Indicates whether OTA updates are enabled.
 */
isEnabled():boolean {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? !!this.bb!.readInt8(this.bb_pos + offset) : false;
}

/**
 * The domain name of the OTA Content Delivery Network (CDN).
 */
cdnDomain():string|null
cdnDomain(optionalEncoding:flatbuffers.Encoding):string|Uint8Array|null
cdnDomain(optionalEncoding?:any):string|Uint8Array|null {
  const offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? this.bb!.__string(this.bb_pos + offset, optionalEncoding) : null;
}

/**
 * The update channel to use, e.g. "stable", "beta", "dev".
 */
updateChannel():string|null
updateChannel(optionalEncoding:flatbuffers.Encoding):string|Uint8Array|null
updateChannel(optionalEncoding?:any):string|Uint8Array|null {
  const offset = this.bb!.__offset(this.bb_pos, 8);
  return offset ? this.bb!.__string(this.bb_pos + offset, optionalEncoding) : null;
}

/**
 * The interval between update checks in minutes, 0 to disable automatic update checks. ( 5 minutes minimum )
 */
checkInterval():number {
  const offset = this.bb!.__offset(this.bb_pos, 10);
  return offset ? this.bb!.readUint16(this.bb_pos + offset) : 0;
}

/**
 * Indicates if the backend is authorized to manage the device's update version on behalf of the user.
 */
allowBackendManagement():boolean {
  const offset = this.bb!.__offset(this.bb_pos, 12);
  return offset ? !!this.bb!.readInt8(this.bb_pos + offset) : false;
}

/**
 * Indicates if manual approval via serial input or captive portal is required before installing updates.
 */
requireManualApproval():boolean {
  const offset = this.bb!.__offset(this.bb_pos, 14);
  return offset ? !!this.bb!.readInt8(this.bb_pos + offset) : false;
}

static startOtaUpdateConfig(builder:flatbuffers.Builder) {
  builder.startObject(6);
}

static addIsEnabled(builder:flatbuffers.Builder, isEnabled:boolean) {
  builder.addFieldInt8(0, +isEnabled, +false);
}

static addCdnDomain(builder:flatbuffers.Builder, cdnDomainOffset:flatbuffers.Offset) {
  builder.addFieldOffset(1, cdnDomainOffset, 0);
}

static addUpdateChannel(builder:flatbuffers.Builder, updateChannelOffset:flatbuffers.Offset) {
  builder.addFieldOffset(2, updateChannelOffset, 0);
}

static addCheckInterval(builder:flatbuffers.Builder, checkInterval:number) {
  builder.addFieldInt16(3, checkInterval, 0);
}

static addAllowBackendManagement(builder:flatbuffers.Builder, allowBackendManagement:boolean) {
  builder.addFieldInt8(4, +allowBackendManagement, +false);
}

static addRequireManualApproval(builder:flatbuffers.Builder, requireManualApproval:boolean) {
  builder.addFieldInt8(5, +requireManualApproval, +false);
}

static endOtaUpdateConfig(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createOtaUpdateConfig(builder:flatbuffers.Builder, isEnabled:boolean, cdnDomainOffset:flatbuffers.Offset, updateChannelOffset:flatbuffers.Offset, checkInterval:number, allowBackendManagement:boolean, requireManualApproval:boolean):flatbuffers.Offset {
  OtaUpdateConfig.startOtaUpdateConfig(builder);
  OtaUpdateConfig.addIsEnabled(builder, isEnabled);
  OtaUpdateConfig.addCdnDomain(builder, cdnDomainOffset);
  OtaUpdateConfig.addUpdateChannel(builder, updateChannelOffset);
  OtaUpdateConfig.addCheckInterval(builder, checkInterval);
  OtaUpdateConfig.addAllowBackendManagement(builder, allowBackendManagement);
  OtaUpdateConfig.addRequireManualApproval(builder, requireManualApproval);
  return OtaUpdateConfig.endOtaUpdateConfig(builder);
}
}
