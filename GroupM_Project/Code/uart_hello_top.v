`timescale 1ns / 1ps

module uart_hello_top (
    input  wire clk100mhz,
    input  wire rst,
    input  wire start,
    output wire uart_tx,
    output wire [15:0] leds
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

    reg [3:0] state;
    reg [3:0] idx;

    localparam S_IDLE = 4'd0;
    localparam S_SEND = 4'd1;
    localparam S_DONE = 4'd2;

    function [7:0] msg_char;
        input [3:0] i;
        begin
            case (i)
                4'd0: msg_char = "H";
                4'd1: msg_char = "E";
                4'd2: msg_char = "L";
                4'd3: msg_char = "L";
                4'd4: msg_char = "O";
                4'd5: msg_char = 8'h0D;
                4'd6: msg_char = 8'h0A;
                default: msg_char = 8'h20;
            endcase
        end
    endfunction

    always @(posedge clk100mhz) begin
        if (rst) begin
            state    <= S_IDLE;
            idx      <= 4'd0;
            tx_start <= 1'b0;
            tx_data  <= 8'd0;
        end else begin
            tx_start <= 1'b0;

            case (state)
                S_IDLE: begin
                    if (start_pulse) begin
                        idx   <= 4'd0;
                        state <= S_SEND;
                    end
                end

                S_SEND: begin
                    if (!tx_busy && idx == 0) begin
                        tx_data  <= msg_char(idx);
                        tx_start <= 1'b1;
                        idx      <= idx + 1'b1;
                    end else if (tx_done) begin
                        if (idx < 7) begin
                            tx_data  <= msg_char(idx);
                            tx_start <= 1'b1;
                            idx      <= idx + 1'b1;
                        end else begin
                            state <= S_DONE;
                        end
                    end
                end

                S_DONE: begin
                    if (start_pulse)
                        state <= S_IDLE;
                end
            endcase
        end
    end

    assign leds[15]   = tx_busy;
    assign leds[14]   = (state == S_DONE);
    assign leds[13:0] = 14'd0;

endmodule