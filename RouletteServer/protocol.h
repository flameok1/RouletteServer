#pragma once


enum Protocol {
	LoginRequest = 1,
	BetRequest = 2,
	StartGameRequest = 3,

	LoginResponse = 1001,
	CountDownSync = 1002,
	BetResponse = 1003,
	GameResult = 1004,
};