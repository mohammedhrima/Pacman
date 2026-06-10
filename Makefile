NAME   = pacman
SRC    = pacman.c
CFLAGS = -fsanitize=address -g3
SDL    = `sdl2-config --cflags --libs` -I`sdl2-config --prefix`/include
LIBS   = -lm

all: $(NAME)

$(NAME): $(SRC) header.h
	cc $(CFLAGS) $(SRC) $(SDL) $(LIBS) -o $(NAME)

clean:
	rm -rf *.dSYM

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
