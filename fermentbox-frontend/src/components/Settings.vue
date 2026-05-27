<template>
  <v-list>
    <v-list-item>
      <v-list-item-content>
        <v-card outlined>
          <v-card-title>
            <v-icon color="secondary">mdi-apps</v-icon>
            <span class="card-title">App</span>
          </v-card-title>
          <v-card-text>
            <v-text-field
              v-model="appPassword"
              label="Password"
              :type="showAppPassword ? 'text' : 'password'"
              :append-icon="showAppPassword ? 'mdi-eye' : 'mdi-eye-off'"
              @click:append="showAppPassword = !showAppPassword"
              clearable
            ></v-text-field>
          </v-card-text>
          <v-card-actions>
            <v-spacer></v-spacer>
            <v-btn
              depressed
              color="accent"
              :loading="isSavingAppPassword"
              @click="storeAppPassword"
              >Save Password</v-btn
            >
          </v-card-actions>
          <v-card-text v-if="appSaveSuccess || appSaveError">
            <v-alert
              dense
              outlined
              :type="appSaveError ? 'error' : 'success'"
              class="mb-0"
            >
              {{ appSaveError || appSaveSuccess }}
            </v-alert>
          </v-card-text>
        </v-card>
      </v-list-item-content>
    </v-list-item>
    <v-list-item>
      <v-list-item-content>
        <v-card outlined>
          <v-card-title>
            <v-icon color="secondary">mdi-wifi</v-icon>
            <span class="card-title">Wi-Fi</span>
          </v-card-title>
          <v-card-text>
            <v-text-field v-model="wifiSSID" label="SSID" clearable>{{
              wifiSSID
            }}</v-text-field>
            <v-text-field
              v-model="wifiPassword"
              label="Password"
              :type="showWifiPassword ? 'text' : 'password'"
              :append-icon="showWifiPassword ? 'mdi-eye' : 'mdi-eye-off'"
              @click:append="showWifiPassword = !showWifiPassword"
              clearable
            >
              {{ wifiPassword }}
            </v-text-field>
          </v-card-text>
          <v-card-actions>
            <v-spacer></v-spacer>
            <v-btn
              depressed
              color="accent"
              :loading="isSavingWifiConfig"
              @click="storeWifiConfig"
              >Save WIFI Credentials</v-btn
            >
          </v-card-actions>
          <v-card-text v-if="wifiSaveSuccess || wifiSaveError">
            <v-alert
              dense
              outlined
              :type="wifiSaveError ? 'error' : 'success'"
              class="mb-0"
            >
              {{ wifiSaveError || wifiSaveSuccess }}
            </v-alert>
          </v-card-text>
        </v-card>
      </v-list-item-content>
    </v-list-item>
    <v-list-item>
      <v-list-item-content>
        <v-card outlined>
          <v-card-title>
            <v-icon color="secondary">mdi-flask-outline</v-icon>
            <span class="card-title">Simulation</span>
          </v-card-title>
          <v-card-text>
            <v-switch
              v-model="fakeMode"
              label="Enable fake sensors and actors"
              inset
            ></v-switch>
          </v-card-text>
          <v-card-actions>
            <v-spacer></v-spacer>
            <v-btn
              depressed
              color="accent"
              :loading="isSavingFakeMode"
              @click="storeFakeMode"
              >Save Simulation Mode</v-btn
            >
          </v-card-actions>
          <v-card-text v-if="fakeModeSaveSuccess || fakeModeSaveError">
            <v-alert
              dense
              outlined
              :type="fakeModeSaveError ? 'error' : 'success'"
              class="mb-0"
            >
              {{ fakeModeSaveError || fakeModeSaveSuccess }}
            </v-alert>
          </v-card-text>
        </v-card>
      </v-list-item-content>
    </v-list-item>
  </v-list>
</template>

<style lang="scss" scoped>
@import "../style/styles.scss";
.page-title {
  color: $color-accent;
  margin-left: 5px;
  font-size: 1.4em !important;
}
.card-title {
  color: $color-secondary;
  margin-left: 5px;
}
</style>

<script>
import apiService from "../services/api.js";

export default {
  name: "Settings",

  data: () => ({
    appPassword: null,
    showAppPassword: false,
    isSavingAppPassword: false,
    appSaveSuccess: "",
    appSaveError: "",
    wifiSSID: null,
    wifiPassword: null,
    showWifiPassword: false,
    isSavingWifiConfig: false,
    wifiSaveSuccess: "",
    wifiSaveError: "",
    fakeMode: false,
    isSavingFakeMode: false,
    fakeModeSaveSuccess: "",
    fakeModeSaveError: "",
  }),
  created: function () {
    this.getConfig();
  },
  methods: {
    storeAppPassword: function () {
      const ssid = (this.wifiSSID || "").trim();
      const password = (this.appPassword || "").trim();

      this.appSaveSuccess = "";
      this.appSaveError = "";

      if (!ssid || !password) {
        this.appSaveError = "SSID and password are required.";
        return Promise.resolve();
      }

      this.isSavingAppPassword = true;
      return apiService
        .updateNetworkConfig({ SSID: ssid, Password: password })
        .then(() => this.getConfig())
        .then(() => {
          this.appSaveSuccess = "Password saved.";
        })
        .catch(() => {
          this.appSaveError = "Unable to save password. Please try again.";
        })
        .finally(() => {
          this.isSavingAppPassword = false;
        });
    },
    storeWifiConfig: function () {
      const ssid = (this.wifiSSID || "").trim();
      const password = (this.wifiPassword || "").trim();

      this.wifiSaveSuccess = "";
      this.wifiSaveError = "";

      if (!ssid || !password) {
        this.wifiSaveError = "SSID and password are required.";
        return Promise.resolve();
      }

      this.isSavingWifiConfig = true;
      return apiService
        .updateNetworkConfig({ SSID: ssid, Password: password })
        .then(() => this.getConfig())
        .then(() => {
          this.wifiSaveSuccess = "Wi-Fi credentials saved.";
        })
        .catch(() => {
          this.wifiSaveError =
            "Unable to save Wi-Fi credentials. Please try again.";
        })
        .finally(() => {
          this.isSavingWifiConfig = false;
        });
    },
    storeFakeMode: function () {
      this.fakeModeSaveSuccess = "";
      this.fakeModeSaveError = "";
      this.isSavingFakeMode = true;

      return apiService
        .updateNetworkConfig({ FakeMode: !!this.fakeMode })
        .then(() => this.getConfig())
        .then(() => {
          this.fakeModeSaveSuccess = "Simulation mode saved.";
        })
        .catch(() => {
          this.fakeModeSaveError = "Unable to save simulation mode.";
        })
        .finally(() => {
          this.isSavingFakeMode = false;
        });
    },
    getConfig: function () {
      return apiService.getConfig().then((data) => {
        this.wifiSSID = data.Wifi.SSID;
        this.wifiPassword = data.Wifi.Password;
        this.appPassword = data.Wifi.Password;
        this.fakeMode = !!data.FakeMode;
      });
    },
  },
};
</script>
