#include "./include/pixel_matrix.hpp"

const int MAX_ARGS = 5; //current max options is three

int main(int argc, char *argv[])
{
	//get command line args
	//input output transformations
	if (argc < 4)
	{
		//error, no transformation applied
		std::cout << "Error: No transformations given! \n";
		std::cout << "Program requires command line args that specify: <input_file> <output_file> < <tranformations> >\n";
		return 0;
	}

	//create pixel obj with input image file
	pixels::pixel_matrix pixelator(argv[1]);
	if (pixelator.is_empty())
	{
		std::cout << argv[1] << "\n";
		std::cout << "Error: Given empty or non-existent input file!\n";
		return 0;
	}

	//call class parser to set chosen options
	for (int i = 3; i < argc; i++)
	{
		if (i <= MAX_ARGS)
		{
			if (!pixelator.set_options(std::string(argv[i])))
			{
				std::cout << "Error: Encountered an invalid transformation\n";
				return 0;
			}
		}
		else
		{
			std::cout << "Warning: Too many transformations given, only first " << MAX_ARGS - 2 << " were applied\n";
			break;
		}
	}

	//apply options
	pixelator.apply_options();

	//output final object
	pixelator.write_to_file(argv[2]);

	return 0;
}