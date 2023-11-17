/**
 * @brief Interpolable Look-up Table. It provides the ability to store a look-up table 
 *      with reference values for the x and y directions that are used for interpolation.
 * 
 * @details The purpose is to be used to perform calculations based on models that don't 
 *      adhere to simpler mathamatical functions. For example, compensating for temperature
 *      in pH measurements varies by temperature. The higher the temperature and pH, the more 
 *      impactful temperature is on the measurement.
 * 
 *      Thus, the calculation done here 'normalizes' or 'standardizes' x_input value based
 *      on the y_input value in the find() function. In the case of pH for example, the
 *      input of x and y would be pH and temperature respectively. The output would be the
 *      pH at a standardized temperature (typically 25 degrees Celsius for pH) as represented by
 *      the x-reference list
 * 
 * @author Athly
*/

#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <vector>
#include <iostream>
#include <iomanip>

class InterpolableLUT {

    public:
        InterpolableLUT(const std::vector<std::vector<double>> (&table), 
                        const std::vector<double> x_ref, 
                        const std::vector<double> y_ref,
                        int x_len,
                        int y_len):table(table), x_ref(x_ref), y_ref(y_ref), x_len(x_len), y_len(y_len) { }
        
        ~InterpolableLUT() { }
    
        /**
         * @brief Calculates the standardized (based on the reference lists) value for x_input
         * @details Uses interpolation in the y-direction to create a temporary 'row' in the
         *      x-direction
        */
        double find(double x_input, double y_input) {
            int x_lower_idx = 0;
            int x_upper_idx = 0;
            int y_lower_idx = 0;
            int y_upper_idx = 0;
            double result = (double)x_input;
            
            if (find_nearest_indexes(y_ref, y_len, y_input, &y_lower_idx, &y_upper_idx)) {
                // Interpolate the pH values at the measured temperature for each buffer.
                std::vector<double> interpolated_y_values_at_x(x_len);
                for (int i = 0; i < x_len; i++) {
                    interpolated_y_values_at_x[i] = linear_interpolate(y_ref[y_lower_idx], table[y_lower_idx][i],
                                                                       y_ref[y_upper_idx], table[y_upper_idx][i],
                                                                       y_input);
                }
                
                if (find_nearest_indexes(interpolated_y_values_at_x, x_len, x_input, &x_lower_idx, &x_upper_idx)) {
                    result = linear_interpolate(interpolated_y_values_at_x[x_lower_idx], x_ref[x_lower_idx],
                                                interpolated_y_values_at_x[x_upper_idx], x_ref[x_upper_idx],
                                                x_input);
                }
            }

            return result;
        }

        /**
         * @brief Provides read-only access to the y-reference values
        */
        const std::vector<double> getYRef() const {
            return y_ref;
        }
    
        /**
         * @brief Provides read-only access to the table
        */
        const std::vector<double> operator[](size_t row) const {
            if (row >= y_len) {
                throw std::out_of_range("Row index out of range");
            }
            return table[row];
        }
    
    private:
        int x_len;  ///< Size of the table int the x-direction
        int y_len;  ///< Size of the table int the x-direction
        std::vector<std::vector<double>> table; ///< Storage of the look-up table
        std::vector<double> x_ref;  ///< Interpolation reference values for the x-direction 
        std::vector<double> y_ref;  ///< Interpolation reference values for the y-direction
        
        /**
         * @brief Find the two indexes in the list where the search_val would fall in between
         * @return Indexes via pointers lower_result and upper_result
         *         true if the search was successful, false if the search value is out of range of the list
        */
        bool find_nearest_indexes(std::vector<double> list, int list_len, double search_val, int *lower_result, int *upper_result) {
            bool ret = false;
            for (int i = 0; i < list_len-1; i++) {
                if ((list[i] <= search_val) && (list[i+1] > search_val)) {
                    *lower_result = i;
                    *upper_result = i + 1;
                    ret = true;
                    break;
                }
            }
            return ret;
        }
        
        // This function performs linear interpolation between two points.
        double linear_interpolate(double x0, double y0, double x1, double y1, double x) {
            return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
        }
};

std::ostream& operator<<(std::ostream& os, const InterpolableLUT &table) {
    const std::vector<double> y_ref = table.getYRef();
    for (int i = 0; i < y_ref.size(); i++) {
        os << std::fixed << std::setprecision(2) << y_ref[i] << "\t";
        for (int j = 0; j < table[i].size(); j++) {
            os << std::fixed << std::setprecision(2) << table[i][j] << "\t";
        }
        os << "\n";
    }
    return os;
}

/******************************************************************************
                Example for the InterpolateLLUT class
*******************************************************************************/
#define NUM_TEMP_POINTS 12
#define NUM_PH_POINTS 7

void example() {
    const std::vector<double> temp_points= {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55};
    const std::vector<double> ph_values_at_25= {1.68, 4.01, 6.86, 7.00, 9.18, 10.01, 12.46};

    // pH values at each temperature point for each pH buffer
    const std::vector<std::vector<double>> ph_values = {
        {1.67, 4.01, 6.98, 7.12, 9.46, 10.32, 13.47}, // 0°C
        {1.67, 4.01, 6.95, 7.09, 9.39, 10.25, 13.25}, // 5°C
        {1.67, 4.00, 6.92, 7.06, 9.32, 10.18, 13.03}, // 10°C
        {1.67, 4.00, 6.90, 7.04, 9.27, 10.12, 12.83}, // 15°C
        {1.68, 4.00, 6.88, 7.02, 9.22, 10.06, 12.64}, // 20°C
        {1.68, 4.01, 6.86, 7.00, 9.18, 10.01, 12.46}, // 25°C
        {1.69, 4.01, 6.85, 6.98, 9.14,  9.97, 12.29}, // 30°C
        {1.69, 4.02, 6.84, 6.98, 9.10,  9.93, 12.14}, // 35°C
        {1.70, 4.03, 6.84, 6.97, 9.07,  9.89, 11.99}, // 40°C
        {1.70, 4.04, 6.83, 6.97, 9.04,  9.86, 11.86}, // 45°C
        {1.71, 4.06, 6.83, 6.97, 9.01,  9.83, 11.73}, // 50°C
        {1.72, 4.08, 6.83, 6.97, 8.99,  9.81, 11.61}  // 55°C
    };
    
    // Initialize the object with the table and lists above 
    InterpolableLUT lutPh(ph_values, ph_values_at_25, temp_points, NUM_PH_POINTS, NUM_TEMP_POINTS);    
    
    printf("pH: %.2f\n", lutPh.find(7.01, 37.0));
    printf("pH: %.2f\n", lutPh.find(7.50, 37.0));
    printf("pH: %.2f\n", lutPh.find(8.00, 37.0));
    printf("pH: %.2f\n", lutPh.find(8.50, 37.0));
    printf("pH: %.2f\n", lutPh.find(9.00, 37.0));
    printf("pH: %.2f\n", lutPh.find(10.01, 0.01));

    std::cout << lutPh;
}