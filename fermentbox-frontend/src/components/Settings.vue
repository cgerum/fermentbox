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
              :type="showAppPassword ? 'text': 'password'"
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
            <v-text-field v-model="wifiName" label="Name" clearable></v-text-field>
            <v-text-field
              v-model="wifiPassword"
              label="Password"
              :type="showWifiPassword ? 'text': 'password'"
              :append-icon="showWifiPassword ? 'mdi-eye' : 'mdi-eye-off'"
              @click:append="showWifiPassword = !showWifiPassword"
              clearable
            ></v-text-field>
          </v-card-text>
          <v-card-actions>
            <v-spacer></v-spacer>
            <v-btn depressed color="accent">Save Credentials</v-btn>
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
export default {
  name: "Settings",

  data: () => ({
    appPassword: null,
    showAppPassword: false,
    wifiName: null,
    wifiPassword: null,
    showWifiPassword: false
  }),
  created: function() {
    this.loadAppPassword();
    this.loadWifiName();
    this.loadWifiPassword();
  },
  methods: {
    loadAppPassword: function() {
      // TODO load app password
    },
    storeAppPassword: function() {
      // TODO store app password
    },
    loadWifiName: function() {
      // TODO load wifi name
    },
    storeWifiName: function() {
      // TODO store wifi name
    },
    loadWifiPassword: function() {
      // TODO load wifi password
    },
    storeWifiPassword: function() {
      // TODO store wifi password
    }
  }
};
</script>
