`timescale 1ns / 1ps

module aes_uart_demo_top (
    input  wire clk100mhz,
    input  wire rst,
    input  wire start,
    output wire uart_tx,
    output wire [15:0] leds
);

    localparam [127:0] PLAINTEXT   = 128'h00112233445566778899AABBCCDDEEFF;
    localparam [127:0] KEY         = 128'h000102030405060708090A0B0C0D0E0F;
    localparam [127:0] EXPECT_CIPH = 128'h69C4E0D86A7B0430D8CDB78070B4C55A;

    reg         aes_start;
    reg         aes_enc_dec;
    reg [127:0] aes_data_in;
    wire [127:0] aes_data_out;
    wire        aes_busy;
    wire        aes_done;

    aes128_core u_core (
        .clk      (clk100mhz),
        .rst      (rst),
        .start    (aes_start),
        .enc_dec  (aes_enc_dec),
        .key      (KEY),
        .data_in  (aes_data_in),
        .data_out (aes_data_out),
        .busy     (aes_busy),
        .done     (aes_done)
    );

    reg        tx_start;
    reg [7:0]  tx_data;
    wire       tx_busy;
    wire       tx_done;

    uart_tx #(
        .CLK_FREQ_HZ(100_000_000),
        .BAUD_RATE  (115200)
    ) u_uart (
        .clk    (clk100mhz),
        .rst    (rst),
        .start  (tx_start),
        .data_in(tx_data),
        .tx     (uart_tx),
        .busy   (tx_busy),
        .done   (tx_done)
    );

    reg start_d;
    wire start_pulse;

    always @(posedge clk100mhz) begin
        if (rst)
            start_d <= 1'b0;
        else
            start_d <= start;
    end

    assign start_pulse = start & ~start_d;

    reg [127:0] cipher_reg;
    reg [127:0] decrypt_reg;
    reg [127:0] current_block;

    reg [7:0] label_idx;
    reg [5:0] hex_idx;
    reg [2:0] line_sel;
    reg [3:0] state;

    localparam S_IDLE       = 4'd0;
    localparam S_START_ENC  = 4'd1;
    localparam S_WAIT_ENC   = 4'd2;
    localparam S_START_DEC  = 4'd3;
    localparam S_WAIT_DEC   = 4'd4;
    localparam S_PREP_LINE  = 4'd5;
    localparam S_SEND_LABEL = 4'd6;
    localparam S_SEND_HEX   = 4'd7;
    localparam S_SEND_CR    = 4'd8;
    localparam S_SEND_LF    = 4'd9;
    localparam S_NEXT_LINE  = 4'd10;
    localparam S_DONE       = 4'd11;

    function [7:0] hexchar;
        input [3:0] nib;
        begin
            case (nib)
                4'h0: hexchar = "0";
                4'h1: hexchar = "1";
                4'h2: hexchar = "2";
                4'h3: hexchar = "3";
                4'h4: hexchar = "4";
                4'h5: hexchar = "5";
                4'h6: hexchar = "6";
                4'h7: hexchar = "7";
                4'h8: hexchar = "8";
                4'h9: hexchar = "9";
                4'hA: hexchar = "A";
                4'hB: hexchar = "B";
                4'hC: hexchar = "C";
                4'hD: hexchar = "D";
                4'hE: hexchar = "E";
                4'hF: hexchar = "F";
            endcase
        end
    endfunction

    function [7:0] get_label_char;
        input [2:0] which;
        input [7:0] idx;
        begin
            case (which)
                3'd0: begin
                    case (idx)
                        0:  get_label_char = "P";
                        1:  get_label_char = "L";
                        2:  get_label_char = "A";
                        3:  get_label_char = "I";
                        4:  get_label_char = "N";
                        5:  get_label_char = "T";
                        6:  get_label_char = "E";
                        7:  get_label_char = "X";
                        8:  get_label_char = "T";
                        9:  get_label_char = " ";
                        10: get_label_char = ":";
                        11: get_label_char = " ";
                        default: get_label_char = " ";
                    endcase
                end
                3'd1: begin
                    case (idx)
                        0:  get_label_char = "K";
                        1:  get_label_char = "E";
                        2:  get_label_char = "Y";
                        3:  get_label_char = " ";
                        4:  get_label_char = " ";
                        5:  get_label_char = " ";
                        6:  get_label_char = " ";
                        7:  get_label_char = " ";
                        8:  get_label_char = " ";
                        9:  get_label_char = " ";
                        10: get_label_char = ":";
                        11: get_label_char = " ";
                        default: get_label_char = " ";
                    endcase
                end
                3'd2: begin
                    case (idx)
                        0:  get_label_char = "C";
                        1:  get_label_char = "I";
                        2:  get_label_char = "P";
                        3:  get_label_char = "H";
                        4:  get_label_char = "E";
                        5:  get_label_char = "R";
                        6:  get_label_char = "T";
                        7:  get_label_char = "E";
                        8:  get_label_char = "X";
                        9:  get_label_char = "T";
                        10: get_label_char = ":";
                        11: get_label_char = " ";
                        default: get_label_char = " ";
                    endcase
                end
                3'd3: begin
                    case (idx)
                        0:  get_label_char = "D";
                        1:  get_label_char = "E";
                        2:  get_label_char = "C";
                        3:  get_label_char = "R";
                        4:  get_label_char = "Y";
                        5:  get_label_char = "P";
                        6:  get_label_char = "T";
                        7:  get_label_char = "E";
                        8:  get_label_char = "D";
                        9:  get_label_char = " ";
                        10: get_label_char = ":";
                        11: get_label_char = " ";
                        default: get_label_char = " ";
                    endcase
                end
                default: get_label_char = " ";
            endcase
        end
    endfunction

    function [7:0] get_hex_char;
        input [127:0] block;
        input [5:0] idx;
        begin
            case (idx)
                0:  get_hex_char = hexchar(block[127:124]);
                1:  get_hex_char = hexchar(block[123:120]);
                2:  get_hex_char = hexchar(block[119:116]);
                3:  get_hex_char = hexchar(block[115:112]);
                4:  get_hex_char = hexchar(block[111:108]);
                5:  get_hex_char = hexchar(block[107:104]);
                6:  get_hex_char = hexchar(block[103:100]);
                7:  get_hex_char = hexchar(block[99:96]);
                8:  get_hex_char = hexchar(block[95:92]);
                9:  get_hex_char = hexchar(block[91:88]);
                10: get_hex_char = hexchar(block[87:84]);
                11: get_hex_char = hexchar(block[83:80]);
                12: get_hex_char = hexchar(block[79:76]);
                13: get_hex_char = hexchar(block[75:72]);
                14: get_hex_char = hexchar(block[71:68]);
                15: get_hex_char = hexchar(block[67:64]);
                16: get_hex_char = hexchar(block[63:60]);
                17: get_hex_char = hexchar(block[59:56]);
                18: get_hex_char = hexchar(block[55:52]);
                19: get_hex_char = hexchar(block[51:48]);
                20: get_hex_char = hexchar(block[47:44]);
                21: get_hex_char = hexchar(block[43:40]);
                22: get_hex_char = hexchar(block[39:36]);
                23: get_hex_char = hexchar(block[35:32]);
                24: get_hex_char = hexchar(block[31:28]);
                25: get_hex_char = hexchar(block[27:24]);
                26: get_hex_char = hexchar(block[23:20]);
                27: get_hex_char = hexchar(block[19:16]);
                28: get_hex_char = hexchar(block[15:12]);
                29: get_hex_char = hexchar(block[11:8]);
                30: get_hex_char = hexchar(block[7:4]);
                31: get_hex_char = hexchar(block[3:0]);
                default: get_hex_char = "0";
            endcase
        end
    endfunction

    always @(posedge clk100mhz) begin
        if (rst) begin
            aes_start     <= 1'b0;
            aes_enc_dec   <= 1'b1;
            aes_data_in   <= 128'd0;
            cipher_reg    <= 128'd0;
            decrypt_reg   <= 128'd0;
            current_block <= 128'd0;
            tx_start      <= 1'b0;
            tx_data       <= 8'd0;
            label_idx     <= 8'd0;
            hex_idx       <= 6'd0;
            line_sel      <= 3'd0;
            state         <= S_IDLE;
        end else begin
            aes_start <= 1'b0;
            tx_start  <= 1'b0;

            case (state)
                S_IDLE: begin
                    if (start_pulse) begin
                        aes_enc_dec <= 1'b1;
                        aes_data_in <= PLAINTEXT;
                        state       <= S_START_ENC;
                    end
                end

                S_START_ENC: begin
                    aes_start <= 1'b1;
                    state     <= S_WAIT_ENC;
                end

                S_WAIT_ENC: begin
                    if (aes_done) begin
                        cipher_reg  <= aes_data_out;
                        aes_enc_dec <= 1'b0;
                        aes_data_in <= aes_data_out;
                        state       <= S_START_DEC;
                    end
                end

                S_START_DEC: begin
                    aes_start <= 1'b1;
                    state     <= S_WAIT_DEC;
                end

                S_WAIT_DEC: begin
                    if (aes_done) begin
                        decrypt_reg <= aes_data_out;
                        line_sel    <= 3'd0;
                        state       <= S_PREP_LINE;
                    end
                end

                S_PREP_LINE: begin
                    label_idx <= 8'd0;
                    hex_idx   <= 6'd0;

                    case (line_sel)
                        3'd0: current_block <= PLAINTEXT;
                        3'd1: current_block <= KEY;
                        3'd2: current_block <= cipher_reg;
                        3'd3: current_block <= decrypt_reg;
                        default: current_block <= 128'd0;
                    endcase

                    state <= S_SEND_LABEL;
                end

                S_SEND_LABEL: begin
                    if (!tx_busy && label_idx == 0) begin
                        tx_data   <= get_label_char(line_sel, label_idx);
                        tx_start  <= 1'b1;
                        label_idx <= label_idx + 1'b1;
                    end else if (tx_done) begin
                        if (label_idx < 12) begin
                            tx_data   <= get_label_char(line_sel, label_idx);
                            tx_start  <= 1'b1;
                            label_idx <= label_idx + 1'b1;
                        end else begin
                            state <= S_SEND_HEX;
                        end
                    end
                end

                S_SEND_HEX: begin
                    if (!tx_busy && hex_idx == 0) begin
                        tx_data  <= get_hex_char(current_block, hex_idx);
                        tx_start <= 1'b1;
                        hex_idx  <= hex_idx + 1'b1;
                    end else if (tx_done) begin
                        if (hex_idx < 32) begin
                            tx_data  <= get_hex_char(current_block, hex_idx);
                            tx_start <= 1'b1;
                            hex_idx  <= hex_idx + 1'b1;
                        end else begin
                            state <= S_SEND_CR;
                        end
                    end
                end

                S_SEND_CR: begin
                    if (!tx_busy) begin
                        tx_data  <= 8'h0D;
                        tx_start <= 1'b1;
                        state    <= S_SEND_LF;
                    end
                end

                S_SEND_LF: begin
                    if (tx_done) begin
                        tx_data  <= 8'h0A;
                        tx_start <= 1'b1;
                        state    <= S_NEXT_LINE;
                    end
                end

                S_NEXT_LINE: begin
                    if (tx_done) begin
                        if (line_sel == 3'd3)
                            state <= S_DONE;
                        else begin
                            line_sel <= line_sel + 1'b1;
                            state    <= S_PREP_LINE;
                        end
                    end
                end

                S_DONE: begin
                    if (start_pulse) begin
                        aes_enc_dec <= 1'b1;
                        aes_data_in <= PLAINTEXT;
                        state       <= S_START_ENC;
                    end
                end

                default: state <= S_IDLE;
            endcase
        end
    end

    wire pass = (cipher_reg == EXPECT_CIPH) && (decrypt_reg == PLAINTEXT);

    assign leds[15]   = pass;
    assign leds[14]   = aes_busy;
    assign leds[13]   = (state == S_DONE);
    assign leds[12:0] = cipher_reg[12:0];

endmodule