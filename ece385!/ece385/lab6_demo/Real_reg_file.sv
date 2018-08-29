module reg_file(
					input Clk, LD_REG, Reset,
 					input logic [2:0] SR_1,SR_2,DR,  
					input logic [15:0] bus, 
 					output logic [15:0] SR1_OUT, SR2_OUT 

					);

logic[15:0] R0, R1, R2, R3, R4, R5, R6, R7;



	always_ff @(posedge Clk)
	begin
		if(Reset)
	begin
		R0 <= 16'b0;
		R1 <= 16'b0;
		R2 <= 16'b0;
		R3 <= 16'b0;
		R4 <= 16'b0;
		R5 <= 16'b0;
		R6 <= 16'b0;
		R7 <= 16'b0;
	end
		
		
			if(LD_REG)
				begin
							 
									case(DR)
										 3'b000:      R0<=bus;
										 3'b001:      R1<=bus;
										 3'b010:      R2<=bus;
										 3'b011:      R3<=bus;
										 3'b100:      R4<=bus;
										 3'b101:      R5<=bus;
										 3'b110:      R6<=bus;
										 3'b111:      R7<=bus;
										 		
											
									endcase				
										 
							end 				 
     end     
always_comb
begin
									case(SR_1)
										 3'b000: 	    SR1_OUT = R0;
										 3'b001:        SR1_OUT = R1;
		                         3'b010:        SR1_OUT = R2;		
		                         3'b011: 		 SR1_OUT = R3;
										 3'b100:        SR1_OUT = R4;
										 3'b101:        SR1_OUT = R5; 
                               3'b110:        SR1_OUT = R6;
                               3'b111:        SR1_OUT = R7; 
									endcase
							
							
									case(SR_2)	 
										 3'b000: 	    SR2_OUT = R0; 				
										 3'b001:        SR2_OUT = R1; 
		                         3'b010:        SR2_OUT = R2; 
		                         3'b011: 		 SR2_OUT = R3; 
										 3'b100:        SR2_OUT = R4; 
										 3'b101:        SR2_OUT = R5; 
                               3'b110:        SR2_OUT = R6; 
                               3'b111:        SR2_OUT = R7;
										 
									endcase	 

end
	  
	  
endmodule


