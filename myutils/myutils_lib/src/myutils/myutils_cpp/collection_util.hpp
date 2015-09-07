#ifndef MYUTILS_MYUTILS_CPP_COLLECTION_UTIL_HPP_
#define MYUTILS_MYUTILS_CPP_COLLECTION_UTIL_HPP_

namespace my {
class collection {
public:
	/**
	 * creates a vector of string from command line parameters
	 * @param argc command line argc
	 * @param argv command line argv
	 * @return a list of strings
	 */
	static std::vector<std::string> args_to_vector(int argc, char **argv);

	/**
	 *
	 * @param args
	 * @return command line
	 */
	static std::string args_getBinaryName(std::vector<std::string> &args);

	/**
	 *
	 * @param args list of strings
	 * @param current_position reference to current_position
	 * @return value in current position and increments current_position
	 */
	static std::string next_arg(std::vector<std::string> &args,
			unsigned int &current_position);
	/**
	 *
	 * @param args
	 * @param current_position
	 * @return the integer value for next arg
	 */
	static long long next_arg_int(std::vector<std::string> &args,
			unsigned int &current_position);
	/**
	 *
	 * @param args
	 * @param current_position
	 * @return the double value for next arg
	 */
	static double next_arg_double(std::vector<std::string> &args,
			unsigned int &current_position);

	/**
	 *
	 * @param expected_arg
	 * @param args
	 * @param current_position
	 * @return true if next arg is equal to expected_arg
	 */
	static bool is_next_arg_equal(std::string expected_arg,
			std::vector<std::string> &args, unsigned int &current_position);
	/**
	 *
	 * @param args
	 * @param current_position
	 * @return true if next arg is starts with expected_arg.
	 */
	static bool is_next_arg_startsWith(std::string expected_prefix,
			std::vector<std::string> &args, unsigned int current_position);

	template<typename T_input, typename A_input, typename T_output,
			typename A_output> static void normalizeL1(
			std::vector<T_input, A_input> const &list_input,
			std::vector<T_output, A_output> &list_output,
			const T_output new_sum) {
		double vec_sum = 0;
		for (auto c : list_input) {
			vec_sum += std::abs(c);
		}
		list_output.clear();
		if (vec_sum == 0) {
			list_output.resize(list_input.size(), 0);
		} else {
			for (auto c : list_input) {
				list_output.push_back(
						(T_output) (((double) c / vec_sum) * ((double) new_sum)));
			}
		}
	}
	template<typename T, typename A> static void normalizeL1(
			std::vector<T, A> &list_input_output, const T new_sum) {
		double vec_sum = 0;
		for (auto c : list_input_output) {
			vec_sum += std::abs(c);
		}
		if (vec_sum > 0 && ((T) vec_sum) != new_sum) {
			for (auto &c : list_input_output) {
				c = (T) (((double) c / vec_sum) * ((double) new_sum));
			}
		}
	}

	template<typename T, typename A> static void normalizeToMinMax(
			std::vector<T, A> &list_input_output, const T new_min,
			const T new_max) {
		if (new_max < new_min) {
			T tmp = new_min;
			new_min = new_max;
			new_max = tmp;
		}
		bool first = true;
		T vec_min = 0, vec_max = 0;
		for (auto c : list_input_output) {
			if (first) {
				vec_min = vec_max = c;
				first = false;
			} else if (c < vec_min) {
				vec_min = c;
			} else if (c > vec_max) {
				vec_max = c;
			}
		}
		double range_vec = ((double) vec_max) - ((double) vec_min);
		double range_new = ((double) new_max) - ((double) new_min);
		for (auto &c : list_input_output) {
			if (c == vec_min) {
				c = new_min;
			} else if (c == vec_max) {
				c = new_max;
			} else {
				double frac = (((double) c) - ((double) vec_min)) / range_vec
						* range_new;
				c = new_min + ((T) frac);
			}
		}
	}

};

}

#endif
