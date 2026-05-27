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
            <v-btn depressed color="accent">Save Password</v-btn>
          </v-card-actions>
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
            <v-btn depressed color="accent">Save WIFI Credentials</v-btn>
          </v-card-actions>
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
    wifiSSID: null,
    wifiPassword: null,
    showWifiPassword: false,
  }),
  created: function () {
    this.getConfig();
  },
  methods: {
    storeAppPassword: function () {
      // TODO store app password
    },
    getConfig: function () {
      apiService.getConfig().then((data) => {
        this.wifiSSID = data.Wifi.SSID;
        this.wifiPassword = data.Wifi.Password;
      });
    },
  },
};
</script>
