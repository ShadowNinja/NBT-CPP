#ifndef NBT_SERIALIZATION_HEADER
#define NBT_SERIALIZATION_HEADER

namespace NBT {

extern inline void writeBytes(UByte * bytes, const UByte * write, UInt size);
extern inline void writeByte(UByte * bytes, UByte b);
extern inline void writeShort(UByte * bytes, UShort s);
extern inline void writeInt(UByte * bytes, UInt i);
extern inline void writeLong(UByte * bytes, ULong l);
extern inline void writeFloat(UByte * bytes, float f);
extern inline void writeDouble(UByte * bytes, double d);
extern inline void writeString(UByte * bytes, const char * str, UShort size);


extern inline UByte  readByte  (const UByte * bytes);
extern inline UShort readShort (const UByte * bytes);
extern inline UInt   readInt   (const UByte * bytes);
extern inline ULong  readLong  (const UByte * bytes);
extern inline float  readFloat (const UByte * bytes);
extern inline double readDouble(const UByte * bytes);
extern ByteArray  readByteArray(const UByte * bytes, ULong & index);
extern String     readString   (const UByte * bytes, ULong & index);
extern List       readList     (const UByte * bytes, ULong & index);
extern Compound * readCompound (const UByte * bytes, ULong & index);
extern IntArray   readIntArray (const UByte * bytes, ULong & index);

} // namespace NBT

#endif // NBT_SERIALIZATION_HEADER