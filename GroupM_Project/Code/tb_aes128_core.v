`timescale 1ns / 1ps

module tb_aes128_core;
    reg         clk;
    reg         rst;
    reg         start;
    reg         enc_dec;
    reg [127:0] key;
    reg [127:0] data_in;
    wire [127:0] data_out;
    wire        busy;
    wire        done;

    integer cycle_count;

    localparam [127:0] KEY        = 128'h000102030405060708090A0B0C0D0E0F;
    localparam [127:0] PLAINTEXT  = 128'h00112233445566778899AABBCCDDEEFF;
    localparam [127:0] CIPHERTEXT = 128'h69C4E0D86A7B0430D8CDB78070B4C55A;

    aes128_core dut (
        .clk(clk),
        .rst(rst),
        .start(start),
        .enc_dec(enc_dec),
        .key(key),
        .data_in(data_in),
        .data_out(data_out),
        .busy(busy),
        .done(done)
    );

    always #5 clk = ~clk;

    initial begin
        clk = 0;
        rst = 1;
        start = 0;
        enc_dec = 1'b1;
        key = KEY;
        data_in = PLAINTEXT;
        cycle_count = 0;

        #40;
        rst = 0;

        // Encrypt test
        @(posedge clk);
        start = 1;
        @(posedge clk);
        start = 0;
        cycle_count = 0;
        while (!done) begin
            @(posedge clk);
            cycle_count = cycle_count + 1;
        end
        $display("ENC done in %0d cycles", cycle_count);
        $display("ENC result = %h", data_out);
        if (data_out !== CIPHERTEXT) begin
            $display("ERROR: encryption mismatch");
            $fatal;
        end

        // Decrypt test
        @(posedge clk);
        enc_dec = 1'b0;
        data_in = CIPHERTEXT;
        @(posedge clk);
        start = 1;
        @(posedge clk);
        start = 0;
        cycle_count = 0;
        while (!done) begin
            @(posedge clk);
            cycle_count = cycle_count + 1;
        end
        $display("DEC done in %0d cycles", cycle_count);
        $display("DEC result = %h", data_out);
        if (data_out !== PLAINTEXT) begin
            $display("ERROR: decryption mismatch");
            $fatal;
        end

        $display("PASS: AES-128 encrypt/decrypt known-answer test passed.");
        #20;
        $finish;
    end
endmodule

