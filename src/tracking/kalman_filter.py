import numpy as np
import filterpy.kalman as kalman

from src.utils.time_in_millis import time_in_millis

from src.tracking.config_reader import read_kalman_config

class KalmanFilter():
    def __init__(self, pos, vel, pos_sigma=None, vel_sigma=None):
        """Initialize kalman filter
        Inputs:
            pos -- position of object (polar)
            vel -- velocity of object (polar)
            pos_sigma -- uncertainty of position of object (polar)
            vel_sigma -- uncertainty of veloicty of object (polar)
        """

        kalman_config = read_kalman_config()

        self.state = np.append(pos, vel).astype(np.float32)       # create state vector (elements are r, bear, v_r, v_bear)
        if pos_sigma is None:
            pos_sigma = np.array([kalman_config['r_sigma'], kalman_config['theta_sigma']]).astype(np.float32)
        if vel_sigma is None:
            vel_sigma = np.array([kalman_config['r_hat_sigma'], kalman_config['theta_hat_sigma']]).astype(np.float32)
        self.covar = np.diag(np.append(pos_sigma, vel_sigma)).astype(np.float32)   # create covariance matrix (matrix of certainties of measurements)
        self.measurement_covar = np.eye(self.covar.shape[0]).astype(np.float32)

        self.process_noise = np.eye(self.state.shape[0]).astype(np.float32)      # initalize process noise

        self.last_time_changed = time_in_millis()
        self.delta_t = 0

        # create state transition matrix
        self.state_trans = np.array([[1., 0, self.delta_t, 0],
                                    [0, 1., 0, self.delta_t],
                                    [0, 0, 1., 0],
                                    [0, 0, 0, 1.]])

        self.measurement_trans = np.eye(self.state.size)    # create measurement transition matrix

    def predict(self):
        """Predicts next state of object
        Side Effects:
            self.state_trans -- calls _update_trans_matrix which updates transition matrix
            self.state -- updates state through kalman predict
            self.covar -- updates uncertainty matrix through kalman predict
        """
        self._update_trans_matrix()  # update state transition matrix with update delta_t
        self._update_process_noise()
        self.state, self.covar = kalman.predict(x=self.state, P=self.covar, F=self.state_trans, Q=self.process_noise)

        self._adjust_wraparound()

    def update(self, pos, vel):
        """Update object position and filter
        Inputs:
            pos -- position of object (cartesian)
            vel -- veloicty of object (cartesian)
#           hist_score -- certainty score based on object history (used as scale factor for measurement covariance) (range 1 - 1.05)
        """
        measurement = np.append(pos, vel).astype(np.float32)

        self.state, self.covar = kalman.update(x=self.state, P=self.covar, z=measurement, R=self.measurement_covar, H=self.measurement_trans)

    def _update_trans_matrix(self):
        """Updates transition matrix for time delta since last prediction
        Side Effects:
            self.state_trans -- updates velocity coefficients in position equations
            self.last_time_changed -- updates last time changed to reflect that state has changed
            self.delta_t -- updates delta between current time and last time changed (used for predict)
        """
        self.delta_t = (time_in_millis() - self.last_time_changed) / 1000.

        # update delta_t in state transition matrix
        self.state_trans[0, 2] = self.delta_t
        self.state_trans[1, 3] = self.delta_t

        self.last_time_changed = time_in_millis()

    def _update_process_noise(self):
        """
        Updates process noise using distance from origin of object and velocity of obj
        Side Effects:
            self.process_noise -- updates using range and object velocity
        """
        process_noise = np.diag(np.ones(4))

        # bearing noise increases as distance from origin DECREASES (small changes in position result in large bearing changes)
        bearing_scale_fac = 0.5 + 50*(np.power(self.state[0], -2))        # arbitrary choice for numerator
        vel_scale_fac = [1 + (abs(vel)) for vel in self.state[2:4]]
        process_noise[0::2, 0::2] *= self.delta_t * vel_scale_fac[0]
        process_noise[1::2, 1::2] *= bearing_scale_fac * vel_scale_fac[1] * self.delta_t

        self.process_noise = process_noise

    def _adjust_wraparound(self):
        """ 
        Wraps data from -180 to 180
        Side Effects:   
            self.state -- wraps bearing
        """
        if self.state[1] > 180:
            self.state[1] = -180. + (self.state[1] % 180)
