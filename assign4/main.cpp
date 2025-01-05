/*
 * CS106L Assignment 4: Weather Forecast
 * Created by Haven Whitney with modifications by Fabio Ibanez & Jacob Roberts-Baca.
 */

#include <algorithm>
#include <random>
#include <vector>
#include <iostream>


/* #### Please feel free to use these values, but don't change them! #### */
double kMaxTempRequirement = 5;
double uAvgTempRequirement = 60;

struct Forecast {
  double min_temp;
  double max_temp;
  double avg_temp;
};

Forecast compute_forecast(const std::vector<double>& dailyWeather) {
  // STUDENT TODO 1: return a forecast for the daily weather that is passed in.
  auto min_tmp = std::min_element(dailyWeather.begin(), dailyWeather.end());
  auto max_tmp = std::max_element(dailyWeather.begin(), dailyWeather.end());
  auto avg_temp = std::accumulate(dailyWeather.begin(), dailyWeather.end(), 0, [](double a, double b) {return a + b;})/static_cast<double>(dailyWeather.size());
  // std::cout << "Min: " << *min_tmp << " Max: " << *max_tmp << " Avg: " << avg_temp << std::endl;
  return Forecast{*min_tmp, *max_tmp, avg_temp};

}

std::vector<Forecast> get_forecasts(const std::vector<std::vector<double>>& weatherData) {
  /*
   * STUDENT TODO 2: returns a vector of Forecast structs for the weatherData which contains
   * std::vector<double> which contain values for the weather on that day.
   */
  std::vector<Forecast> forecasts;
  std::transform(weatherData.begin(), weatherData.end(), std::back_inserter(forecasts), compute_forecast);
  // for(auto& f : forecasts) {
  //   std::cout << "Min: " << f.min_temp << " Max: " << f.max_temp << " Avg: " << f.avg_temp << std::endl;
  // }
  return forecasts;
}

std::vector<Forecast> get_filtered_data(const std::vector<Forecast>& forecastData) {
  // STUDENT TODO 3: return a vector with Forecasts filtered for days with specific weather.
  std::vector<Forecast> filtered_data;
  filtered_data = forecastData;
  filtered_data.erase(std::remove_if(filtered_data.begin(), filtered_data.end(), [](const Forecast& f) {
    return (f.max_temp - f.min_temp) <= kMaxTempRequirement || f.avg_temp < uAvgTempRequirement;
  }), filtered_data.end());
  return filtered_data;
}


std::vector<Forecast> get_shuffled_data(const std::vector<Forecast>& forecastData) {
  // STUDENT TODO 4: Make use of a standard library algorithm to shuffle the data!
  std::random_device rd;
  std::mt19937 g(rd());
  std::vector<Forecast> shuffled_data = forecastData;
  std::shuffle(shuffled_data.begin(), shuffled_data.end(), g);
  return shuffled_data;
}

std::vector<Forecast> run_weather_pipeline(const std::vector<std::vector<double>>& weatherData) {
  // STUDENT TODO 5: Put your functions together to run the weather pipeline!
  std::vector<Forecast> forecasts = get_forecasts(weatherData);
  std::vector<Forecast> filtered_data = get_filtered_data(forecasts);
  std::vector<Forecast> shuffled_data = get_shuffled_data(filtered_data);
  return shuffled_data;
}

/* #### Please don't change this line! #### */
#include "utils.cpp"