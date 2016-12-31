F_function_vectorized(initialLLR.data(), LLR[0][9].data(), 512);
F_function_vectorized(LLR[0][9].data(), LLR[0][8].data(), 256);
F_function_vectorized(LLR[0][8].data(), LLR[0][7].data(), 128);
Repetition_multiPath(7, 0);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), Bits[currentPath].data()+0, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
Repetition_multiPath(6, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+128, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
Rate0_multiPath(4, 192);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Rate0_multiPath(3, 208);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 216);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+216, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 220);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+220, 2);
Rate1_multiPath(1, 222);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+220, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+216, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+208, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+192, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+192, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 224);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+224, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 232);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+232, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 236);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+236, 2);
Rate1_multiPath(1, 238);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+236, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+232, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+224, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+224, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 240);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+240, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 244);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+244, 2);
Rate1_multiPath(1, 246);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+244, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+240, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+240, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 248);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+248, 2);
Rate1_multiPath(1, 250);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+248, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+248, 4);
Rate1_multiPath(2, 252);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+248, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+240, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+224, 16);
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
Rate0_multiPath(5, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
Rate0_multiPath(4, 288);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 304);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+304, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 312);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+312, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 316);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+316, 2);
Rate1_multiPath(1, 318);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+316, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+312, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+304, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+288, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+256, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+256, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
Repetition_multiPath(4, 320);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+320, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 336);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+336, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 344);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+344, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 348);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+348, 2);
Rate1_multiPath(1, 350);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+348, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+344, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+336, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+320, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+320, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 352);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+352, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 360);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+360, 4);
Rate1_multiPath(2, 364);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+360, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+352, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+352, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 368);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+368, 2);
Rate1_multiPath(1, 370);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+368, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+368, 4);
Rate1_multiPath(2, 372);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+368, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+368, 8);
Rate1_multiPath(3, 376);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+368, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+352, 16);
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
Repetition_multiPath(4, 384);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+384, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Rate0_multiPath(2, 400);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate0_multiPath(1, 404);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate1_multiPath(1, 406);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+404, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+400, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+400, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 408);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+408, 2);
Rate1_multiPath(1, 410);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+408, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+408, 4);
Rate1_multiPath(2, 412);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+408, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+400, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+384, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+384, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 416);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+416, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 420);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+420, 2);
Rate1_multiPath(1, 422);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+420, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+416, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+416, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 424);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+424, 2);
Rate1_multiPath(1, 426);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+424, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+424, 4);
Rate1_multiPath(2, 428);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+424, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+416, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+416, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 432);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+432, 2);
Rate1_multiPath(1, 434);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+432, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+432, 4);
Rate1_multiPath(2, 436);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+432, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+432, 8);
Rate1_multiPath(3, 440);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+432, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+416, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+384, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+384, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 448);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+448, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 452);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+452, 2);
Rate1_multiPath(1, 454);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+452, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+448, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+448, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 456);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+456, 2);
Rate1_multiPath(1, 458);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+456, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+456, 4);
Rate1_multiPath(2, 460);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+456, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+448, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+448, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 464);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+464, 2);
Rate1_multiPath(1, 466);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+464, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+464, 4);
Rate1_multiPath(2, 468);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+464, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+464, 8);
Rate1_multiPath(3, 472);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+464, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+448, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+448, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 480);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+480, 2);
Rate1_multiPath(1, 482);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+480, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+480, 4);
Rate1_multiPath(2, 484);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+480, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+480, 8);
Rate1_multiPath(3, 488);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+480, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+480, 16);
Rate1_multiPath(4, 496);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+480, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+448, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+384, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+256, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+0, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(initialLLR.data(), LLR[currentPath][9].data(), Bits[currentPath].data()+0, 512);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][9].data(), LLR[currentPath][8].data(), 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
Repetition_multiPath(5, 512);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+512, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
Repetition_multiPath(4, 544);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+544, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
Repetition_multiPath(3, 560);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+560, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate0_multiPath(1, 568);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate1_multiPath(1, 570);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+568, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+568, 4);
Rate1_multiPath(2, 572);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+568, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+560, 8);
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
Rate0_multiPath(3, 576);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Rate0_multiPath(2, 584);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate0_multiPath(1, 588);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_0R(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Rate1_multiPath(1, 590);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+588, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+584, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	Combine_0RSimple(Bits[currentPath].data()+576, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+576, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 592);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+592, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 596);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+596, 2);
Rate1_multiPath(1, 598);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+596, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+592, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+592, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 600);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+600, 2);
Rate1_multiPath(1, 602);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+600, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+600, 4);
Rate1_multiPath(2, 604);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+600, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+592, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+576, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+576, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 608);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+608, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 612);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+612, 2);
Rate1_multiPath(1, 614);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+612, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+608, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+608, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 616);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+616, 2);
Rate1_multiPath(1, 618);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+616, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+616, 4);
Rate1_multiPath(2, 620);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+616, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+608, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+608, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 624);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+624, 2);
Rate1_multiPath(1, 626);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+624, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+624, 4);
Rate1_multiPath(2, 628);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+624, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+624, 8);
Rate1_multiPath(3, 632);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+624, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+608, 16);
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
Repetition_multiPath(3, 640);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+640, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 648);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+648, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 652);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+652, 2);
Rate1_multiPath(1, 654);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+652, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+648, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+640, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+640, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 656);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+656, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 660);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+660, 2);
Rate1_multiPath(1, 662);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+660, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+656, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+656, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 664);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+664, 2);
Rate1_multiPath(1, 666);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+664, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+664, 4);
Rate1_multiPath(2, 668);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+664, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+656, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+640, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+640, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 672);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+672, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 676);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+676, 2);
Rate1_multiPath(1, 678);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+676, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+672, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+672, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 680);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+680, 2);
Rate1_multiPath(1, 682);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+680, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+680, 4);
Rate1_multiPath(2, 684);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+680, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+672, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+672, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 688);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+688, 2);
Rate1_multiPath(1, 690);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+688, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+688, 4);
Rate1_multiPath(2, 692);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+688, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+688, 8);
Rate1_multiPath(3, 696);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+688, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+672, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+640, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+640, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 704);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+704, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 708);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+708, 2);
Rate1_multiPath(1, 710);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+708, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+704, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+704, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 712);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+712, 2);
Rate1_multiPath(1, 714);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+712, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+712, 4);
Rate1_multiPath(2, 716);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+712, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+704, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+704, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 720);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+720, 2);
Rate1_multiPath(1, 722);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+720, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+720, 4);
Rate1_multiPath(2, 724);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+720, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+720, 8);
Rate1_multiPath(3, 728);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+720, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+704, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+704, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 736);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+736, 2);
Rate1_multiPath(1, 738);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+736, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+736, 4);
Rate1_multiPath(2, 740);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+736, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+736, 8);
Rate1_multiPath(3, 744);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+736, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+736, 16);
Rate1_multiPath(4, 752);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+736, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+704, 32);
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
Repetition_multiPath(3, 768);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+768, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 776);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+776, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 780);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+780, 2);
Rate1_multiPath(1, 782);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+780, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+776, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+768, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+768, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 784);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+784, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 788);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+788, 2);
Rate1_multiPath(1, 790);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+788, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+784, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+784, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 792);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+792, 2);
Rate1_multiPath(1, 794);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+792, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+792, 4);
Rate1_multiPath(2, 796);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+792, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+784, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+768, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+768, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 800);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+800, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 804);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+804, 2);
Rate1_multiPath(1, 806);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+804, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+800, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+800, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 808);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+808, 2);
Rate1_multiPath(1, 810);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+808, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+808, 4);
Rate1_multiPath(2, 812);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+808, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+800, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+800, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 816);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+816, 2);
Rate1_multiPath(1, 818);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+816, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+816, 4);
Rate1_multiPath(2, 820);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+816, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+816, 8);
Rate1_multiPath(3, 824);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+816, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+800, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+768, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+768, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
Repetition_multiPath(2, 832);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+832, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 836);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+836, 2);
Rate1_multiPath(1, 838);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+836, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+832, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+832, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	F_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), 2);
Repetition_multiPath(1, 840);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][2].data(), LLR[currentPath][1].data(), Bits[currentPath].data()+840, 2);
Rate1_multiPath(1, 842);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+840, 2);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+840, 4);
Rate1_multiPath(2, 844);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+840, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+832, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+832, 16);
Rate1_multiPath(4, 848);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+832, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+832, 32);
Rate1_multiPath(5, 864);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+832, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+768, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][8].data(), LLR[currentPath][7].data(), Bits[currentPath].data()+768, 128);
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
Repetition_multiPath(2, 896);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function(LLR[currentPath][3].data(), LLR[currentPath][2].data(), Bits[currentPath].data()+896, 4);
Rate1_multiPath(2, 900);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple(Bits[currentPath].data()+896, 4);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][4].data(), LLR[currentPath][3].data(), Bits[currentPath].data()+896, 8);
Rate1_multiPath(3, 904);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+896, 8);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][5].data(), LLR[currentPath][4].data(), Bits[currentPath].data()+896, 16);
Rate1_multiPath(4, 912);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+896, 16);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][6].data(), LLR[currentPath][5].data(), Bits[currentPath].data()+896, 32);
Rate1_multiPath(5, 928);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+896, 32);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	G_function_vectorized(LLR[currentPath][7].data(), LLR[currentPath][6].data(), Bits[currentPath].data()+896, 64);
Rate1_multiPath(6, 960);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+896, 64);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+768, 128);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+512, 256);
for(int currentPath=0; currentPath<PathCount; ++currentPath)
	CombineSimple_vectorized(Bits[currentPath].data()+0, 512);
