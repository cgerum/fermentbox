<template>
  <v-app>
    <v-app-bar app color="primary" dark>
      <v-btn icon @click="drawer = !drawer">
        <v-app-bar-nav-icon></v-app-bar-nav-icon>
      </v-btn>
      <div class="d-flex align-center">
        <v-icon large>mdi-bacteria</v-icon>
        <span style="font-size: 2em; margin-left: 5px">Ferment Box</span>
      </div>

      <v-spacer></v-spacer>
    </v-app-bar>

    <v-content>
      <v-alert v-if="!status.ok" type="error">{{ status.message }}</v-alert>
      <v-alert v-else-if="status.detailMessages.length > 0" type="info">
        {{ status.detailMessages.join("; ") }}
      </v-alert>

      <v-navigation-drawer v-model="drawer" absolute temporary>
        <v-list>
          <v-list-item link @click="setPage('controls')">
            <v-list-item-icon>
              <v-icon>mdi-file-table-box-outline</v-icon>
            </v-list-item-icon>
            <v-list-item-content>
              <v-list-item-title>Controls</v-list-item-title>
            </v-list-item-content>
          </v-list-item>
          <v-list-item link @click="setPage('schedule')">
            <v-list-item-icon>
              <v-icon>mdi-timer-outline</v-icon>
            </v-list-item-icon>
            <v-list-item-content>
              <v-list-item-title>Schedule</v-list-item-title>
            </v-list-item-content>
          </v-list-item>
          <v-list-item link @click="setPage('settings')">
            <v-list-item-icon>
              <v-icon>mdi-cogs</v-icon>
            </v-list-item-icon>
            <v-list-item-content>
              <v-list-item-title>Settings</v-list-item-title>
            </v-list-item-content>
          </v-list-item>
        </v-list>
      </v-navigation-drawer>

      <div v-show="page === 'controls'">
        <div class="page-title">
          <v-icon large color="accent" class="page-title-icon"
            >mdi-file-table-box-outline</v-icon
          >
          <span>Controls</span>
        </div>
        <v-list>
          <v-list-item>
            <v-list-item-content>
              <temperature-control></temperature-control>
              <humidity-control></humidity-control>
            </v-list-item-content>
          </v-list-item>
        </v-list>
      </div>

      <div v-show="page === 'schedule'">
        <div class="page-title">
          <v-icon large color="accent" class="page-title-icon"
            >mdi-timer-outline</v-icon
          >
          <span>Schedule</span>
        </div>
        <schedule></schedule>
      </div>

      <div v-show="page === 'settings'">
        <div class="page-title">
          <v-icon large color="accent" class="page-title-icon">mdi-cogs</v-icon>
          <span>Settings</span>
        </div>
        <settings></settings>
      </div>
    </v-content>
  </v-app>
</template>

<style lang="scss" scoped>
@import "./style/styles.scss";
.page-title {
  color: $color-accent;
  font-size: 1.6em;
  font-weight: bold;
  margin-left: 15px;
  margin-top: 15px;
}
.page-title-icon {
  margin-right: 5px;
}
</style>


<script>
import TemperatureControl from "./components/TemperatureControl";
import HumidityControl from "./components/HumidityControl";
import Schedule from "./components/Schedule";
import Settings from "./components/Settings";
import EventBus from "./event-bus.js";
import apiService from "./services/api.js";

export default {
  name: "App",

  components: {
    TemperatureControl,
    HumidityControl,
    Schedule,
    Settings,
  },

  data: () => ({
    status: {
      ok: true,
      message: "",
      code: "normal",
      codes: ["normal"],
      detailMessages: [],
    },
    drawer: false,
    page: "controls",
    measurementTimer: 0,
    statusTimer: 0,
  }),

  created: function () {
    this.startMeasurement();
    this.startMonitor();
  },

  beforeDestroy: function () {
    clearInterval(this.measurementTimer);
    clearInterval(this.statusTimer);
  },

  methods: {
    normalizeStatus(nextStatus) {
      const status = {
        ok: true,
        message: "",
        code: "normal",
        codes: ["normal"],
        ...nextStatus,
      };
      const codeMessages = {
        sensor_failed: "Temperature/Humidity sensor failed",
        network_unavailable: "Network connection unavailable",
        config_missing: "Wi-Fi configuration missing",
        schedule_inactive: "No schedule running",
        control_idle: "Control loop is idle",
      };
      const codes = Array.isArray(status.codes) && status.codes.length > 0
        ? status.codes
        : [status.code];
      status.codes = codes;
      status.detailMessages = codes
        .filter((code) => code !== "normal")
        .map((code) => codeMessages[code])
        .filter(Boolean);
      return status;
    },

    setPage(name) {
      this.page = name;
      this.drawer = false;
    },

    startMeasurement() {
      this.measurements = [];
      this.measurementTimer = setInterval(() => {
        apiService.getMeasurement().then((data) => {
          this.measurements.push(data);
          EventBus.$emit("new-measurement", data);
        });
      }, 2500);
    },

    startMonitor() {
      this.statusTimer = setInterval(() => {
        apiService
          .getStatus()
          .then((data) => {
            console.log(data);
            this.status = this.normalizeStatus(data);
          })
          .catch(() => {
            this.status = this.normalizeStatus({
              ok: false,
              message: "Could not get Status!",
            });
          });
      }, 2000);
    },
  },
};
</script>
