<template>
  <v-card outlined>
    <v-card-title>
      <v-icon x-large color="secondary">mdi-water</v-icon>
      <span class="card-title">
        Humidity
        <span>{{currentValue}}</span> %
      </span>
    </v-card-title>
    <v-card-text>
      <v-sparkline
        :gradient="['#0a4c8a', '#97d5e0']"
        :line-width="1"
        :smooth="16"
        :value="log"
        auto-draw
        stroke-linecap="round"
        class="control"
      ></v-sparkline>
    </v-card-text>
  </v-card>
</template>

<style lang="scss" scoped>
@import "../style/styles.scss";
.card-title {
  color: $color-secondary;
}
</style>

<script>
import EventBus from "../event-bus.js";

export default {
  name: "HumidityControl",

  data: () => ({
    log: [],
    start: 0
  }),
  computed: {
    labels: function() {
      return this.log.map(data => data.label);
    },
    currentValue: function() {
      return this.log.length > 0 ? this.log[this.log.length - 1].value : "--";
    }
  },

  created: function() {
    this.start = Math.round(Date.now() / 1000);
    EventBus.$on("new-measurement", data => {
      this.updateLog(data);
    });
  },

  methods: {
    updateLog: function(data) {
      if (this.log.length >= 180) {
        this.log.shift();
      }
      this.log.push({
        label: data.date - this.start,
        value: data.humidity
      });
    }
  }
};
</script>
