from threading import Thread

from pubsub import pub

from src.buoy_detection.Distance_Calculator import DistanceCalculator
from src.waypoint import Waypoint


class BuoyDetector(Thread):
    def __init__(self, boat):
        super().__init__()
        self.distance_calculator = DistanceCalculator(DRAW_IMAGE=True, camera_numbers=(2, 3))
        self.buoy_location = None
        self.boat = boat

    def run(self):
        """Continuously checks for a buoy"""
        while True:
            try:
                left_frame, disparity_frame = self.distance_calculator.depth_map_calculator.calculateDepthMap()
            except Exception:
                continue

            try:
                buoy_pixel_x, buoy_pixel_y = self.distance_calculator.findBuoyPixels(left_frame)
            except Exception:
                continue

            # Get the disparity value of the pixels found
            disparity_of_buoy = self.distance_calculator.getDisparityValue(disparity_frame, buoy_pixel_x, buoy_pixel_y)

            # Calculate the distance to the buoy using the disparity values
            distance_to_buoy = self.distance_calculator.getDistance(disparity_of_buoy)

            # Get a reading from airmar
            real_bearing_reading_from_airmar = self.boat.current_heading
            get_latitude_from_airmar = self.boat.current_position.lat
            get_longitude_from_airmar = self.boat.current_position.long

            calculated_bearing_to_buoy = self.distance_calculator.getBearingFromxPixel(buoy_pixel_x,
                                                                                       real_bearing_reading_from_airmar)

            estimated_buoy_lat, estimated_buoy_long = \
                self.distance_calculator.getBuoyGPSLocation(get_latitude_from_airmar, get_longitude_from_airmar,
                                                            distance_to_buoy, calculated_bearing_to_buoy)

            pub.sendMessage("buoy detected", location=Waypoint(estimated_buoy_lat, estimated_buoy_long))
