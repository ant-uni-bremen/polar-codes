F_function_vectorized(LLR[0][10].data(), LLR[0][9].data(), 512);
F_function_vectorized(LLR[0][9].data(), LLR[0][8].data(), 256);
F_function_vectorized(LLR[0][8].data(), LLR[0][7].data(), 128);
F_function_vectorized(LLR[0][7].data(), LLR[0][6].data(), 64);
Repetition_multiPath(6, 0);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+0, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
Rate0_multiPath(4, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Rate0_multiPath(3, 80);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Rate0_multiPath(2, 88);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate0_multiPath(1, 92);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate1_multiPath(1, 94);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+92, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+88, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+80, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+64, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+64, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Rate0_multiPath(3, 96);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 104);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+104, 4);
SPC_multiPath(2, 108);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+104, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+96, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+96, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 112);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+112, 4);
SPC_multiPath(2, 116);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+112, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+112, 8);
SPC_multiPath(3, 120);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+112, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+96, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+64, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+0, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), Bits[currentPath].data()+0, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
Rate0_multiPath(4, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 144);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+144, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 152);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+152, 4);
SPC_multiPath(2, 156);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+152, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+144, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+128, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+128, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 160);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+160, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 168);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+168, 4);
SPC_multiPath(2, 172);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+168, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+160, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+160, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 176);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+176, 4);
Rate1_multiPath(2, 180);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+176, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+176, 8);
Rate1_multiPath(3, 184);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+176, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+160, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+128, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+128, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 192);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+192, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate0_multiPath(1, 200);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate1_multiPath(1, 202);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+200, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+200, 4);
Rate1_multiPath(2, 204);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+200, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+192, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+192, 16);
SPC_multiPath(4, 208);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+192, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+192, 32);
SPC_multiPath(5, 224);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+192, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+128, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+0, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][9].data(), LLR[currentPath][8].data(), Bits[currentPath].data()+0, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
Repetition_multiPath(4, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+256, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 272);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+272, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 280);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+280, 4);
Rate1_multiPath(2, 284);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+280, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+272, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+256, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+256, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Rate0_multiPath(2, 288);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
SPC_multiPath(2, 292);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+288, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+288, 8);
SPC_multiPath(3, 296);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+288, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+288, 16);
SPC_multiPath(4, 304);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+288, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+256, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+256, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 320);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+320, 4);
SPC_multiPath(2, 324);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+320, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+320, 8);
SPC_multiPath(3, 328);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+320, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+320, 16);
SPC_multiPath(4, 336);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+320, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+320, 32);
SPC_multiPath(5, 352);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+320, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+256, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), Bits[currentPath].data()+256, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 384);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+384, 4);
SPC_multiPath(2, 388);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+384, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+384, 8);
SPC_multiPath(3, 392);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+384, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+384, 16);
SPC_multiPath(4, 400);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+384, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+384, 32);
SPC_multiPath(5, 416);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+384, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+384, 64);
SPC_multiPath(6, 448);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+384, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+256, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+0, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][10].data(), LLR[currentPath][9].data(), Bits[currentPath].data()+0, 512);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][9].data(), LLR[currentPath][8].data(), 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Rate0_multiPath(3, 512);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Rate0_multiPath(2, 520);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
SPC_multiPath(2, 524);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+520, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+512, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+512, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 528);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+528, 4);
SPC_multiPath(2, 532);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+528, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+528, 8);
SPC_multiPath(3, 536);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+528, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+512, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+512, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 544);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+544, 4);
SPC_multiPath(2, 548);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+544, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+544, 8);
SPC_multiPath(3, 552);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+544, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+544, 16);
SPC_multiPath(4, 560);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+544, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+512, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+512, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 576);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+576, 4);
SPC_multiPath(2, 580);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+576, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+576, 8);
SPC_multiPath(3, 584);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+576, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+576, 16);
SPC_multiPath(4, 592);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+576, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+576, 32);
SPC_multiPath(5, 608);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+576, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+512, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), Bits[currentPath].data()+512, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 640);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+640, 4);
SPC_multiPath(2, 644);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+640, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+640, 8);
SPC_multiPath(3, 648);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+640, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+640, 16);
Rate1_multiPath(4, 656);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+640, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+640, 32);
Rate1_multiPath(5, 672);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+640, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+640, 64);
Rate1_multiPath(6, 704);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+640, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+512, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][9].data(), LLR[currentPath][8].data(), Bits[currentPath].data()+512, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 768);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+768, 4);
Rate1_multiPath(2, 772);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+768, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+768, 8);
Rate1_multiPath(3, 776);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+768, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+768, 16);
Rate1_multiPath(4, 784);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+768, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+768, 32);
Rate1_multiPath(5, 800);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+768, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+768, 64);
Rate1_multiPath(6, 832);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+768, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), Bits[currentPath].data()+768, 128);
Rate1_multiPath(7, 896);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+768, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+512, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+0, 512);
