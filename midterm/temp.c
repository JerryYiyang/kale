HuffmanBitWriter *huffman_bit_writer_init(const char *fname){
    HuffmanBitWriter *writer = (HuffmanBitWriter *)malloc(sizeof(HuffmanBitWriter));
    writer->file = fopen(fname, "wb");
    writer->n_bits = 0;
    writer->byte = 0;
    return writer;
}

void huffman_bit_writer_close(HuffmanBitWriter *writer){
    if (writer->n_bits > 0) {
        writer->byte <<= (7 - writer->n_bits);
        fwrite(&(writer->byte), sizeof(uint8_t), 1, writer->file);
    }
    fclose(writer->file);
    free(writer);
}

void huffman_bit_writer_write_str(HuffmanBitWriter *writer, const char *str){
    fwrite(str, sizeof(char), strlen(str), writer->file);
}

void huffman_bit_writer_write_code(HuffmanBitWriter *writer, const char *code){
    for (int i = 0; i < strlen(code); i++) {
        char bit = code[i];
        if (bit == '1') {
            writer->byte += 1;
        }
        if (writer->n_bits == 7) {
            fwrite(&(writer->byte), sizeof(uint8_t), 1, writer->file);
            writer->byte = 0;
            writer->n_bits = 0;
        } else {
            writer->byte <<= 1;
            writer->n_bits += 1;
        }
    }
}

HuffmanBitReader *huffman_bit_reader_init(const char *fname){
    HuffmanBitReader *reader = (HuffmanBitReader *)malloc(sizeof(HuffmanBitReader));
    reader->file = fopen(fname, "rb");
    reader->n_bits = 0;
    reader->byte = 0;
    reader->mask = 0;
    return reader;
}

void huffman_bit_reader_close(HuffmanBitReader *reader){
    fclose(reader->file);
    free(reader);
}

char *huffman_bit_reader_read_str(HuffmanBitReader *reader){
    char *buffer = NULL;
    size_t buffer_size = 0;
    getline(&buffer, &buffer_size, reader->file);
    return buffer;
}

int huffman_bit_reader_read_bit(HuffmanBitReader *reader){
    if (reader->mask == 0) {
        reader->byte = huffman_bit_reader_read_byte(reader);
        reader->mask = 1 << 7;
    }
    int bit = reader->byte & reader->mask;
    reader->mask >>= 1;
    return bit != 0;
}

uint8_t huffman_bit_reader_read_byte(HuffmanBitReader *reader){
    uint8_t byte;
    fread(&byte, sizeof(uint8_t), 1, reader->file);
    return byte;
}

void huff_bit_writer(char *comp, char *code, char *header){
    HuffmanBitWriter *write = huffman_bit_writer_init(comp);
    huffman_bit_writer_write_str(write, header);
    huffman_bit_writer_write_str(write, "\n");
    huffman_bit_writer_write_code(write, code);
    huffman_bit_writer_close(write);
}


