`timescale 1ns / 1ps

module uart_tx #(
    parameter CLK_FREQ_HZ = 100_000_000,
    parameter BAUD_RATE   = 115200
)(
    input  wire       clk,
    input  wire       rst,
    input  wire       start,
    input  wire [7:0] data_in,
    output reg        tx,
    output reg        busy,
    output reg        done
);

    localparam integer CLKS_PER_BIT = CLK_FREQ_HZ / BAUD_RATE;

    localparam S_IDLE  = 2'd0;
    localparam S_START = 2'd1;
    localparam S_DATA  = 2'd2;
    localparam S_STOP  = 2'd3;

    reg [1:0]  state;
    reg [31:0] clk_count;
    reg [2:0]  bit_index;
    reg [7:0]  data_reg;

    always @(posedge clk) begin
        if (rst) begin
            state     <= S_IDLE;
            clk_count <= 32'd0;
            bit_index <= 3'd0;
            data_reg  <= 8'd0;
            tx        <= 1'b1;
            busy      <= 1'b0;
            done      <= 1'b0;
        end else begin
            done <= 1'b0;

            case (state)
                S_IDLE: begin
                    tx        <= 1'b1;
                    busy      <= 1'b0;
                    clk_count <= 32'd0;
                    bit_index <= 3'd0;

                    if (start) begin
                        busy     <= 1'b1;
                        data_reg <= data_in;
                        state    <= S_START;
                    end
                end

                S_START: begin
                    tx <= 1'b0;
                    if (clk_count == CLKS_PER_BIT - 1) begin
                        clk_count <= 32'd0;
                        state     <= S_DATA;
                    end else begin
                        clk_count <= clk_count + 1'b1;
                    end
                end

                S_DATA: begin
                    tx <= data_reg[bit_index];
                    if (clk_count == CLKS_PER_BIT - 1) begin
                        clk_count <= 32'd0;
                        if (bit_index == 3'd7) begin
                            bit_index <= 3'd0;
                            state     <= S_STOP;
                        end else begin
                            bit_index <= bit_index + 1'b1;
                        end
                    end else begin
                        clk_count <= clk_count + 1'b1;
                    end
                end

                S_STOP: begin
                    tx <= 1'b1;
                    if (clk_count == CLKS_PER_BIT - 1) begin
                        clk_count <= 32'd0;
                        state     <= S_IDLE;
                        busy      <= 1'b0;
                        done      <= 1'b1;
                    end else begin
                        clk_count <= clk_count + 1'b1;
                    end
                end

                default: begin
                    state <= S_IDLE;
                end
            endcase
        end
    end

endmodule
