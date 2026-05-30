# Tom Awesome Little Language Interpreter

The compiler for TALL, a little statically typed language I use for teaching game programming in schools. The language itself is pretty basic, but the compiler includes a graphical library, so managing creating a window and drawing on it is natively supported.

Currently it works on Linux, but I have future plans to make it portable for Windows and MacOS.

## Installation

Just clone the repo, as usual and run:

```
sudo make install
```

This will create the compiler, add it to the `/usr/bin` folder, and you are good to go.

If you want to make the executable locally so you can inspect it or play around, just run:

```
make
```

This will compile the compiler for TALL in a local `bin` folder.

## Usage

Create a `.tll` file, and follow the syntax rules, which I will write some day. Or you could try to compile the demo files!

Once you have your pretty `.tll` file, you just compile it using the executable inside the `bin` folder, like so:

```
./bin/tall my_awesome_file.tll
```

## License

This project is available under either the **MIT License** or **Public Domain**. See [LICENSE](LICENSE) for more info.

## Contribution

Right now is more of a side personal project, but if you find something interesting to add or a bug fix that I missed out, open an issue or submit a PR!
