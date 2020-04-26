<template>
  <v-card outlined>
    <v-card-title>
      <v-icon x-large color="secondary">mdi-water</v-icon>
      <span class="card-title">Humidity</span>
    </v-card-title>
    <v-card-text>
        <v-sparkline
        :gradient="['#0a4c8a', '#97d5e0']"
        :line-width="3"
        :smooth="16"
        :value="humidityLog"
        :labels="humidityLogLabels"
        auto-draw
        stroke-linecap="round"
        class="control"
        ></v-sparkline>
        <v-slider
        v-model="humidity"
        class="align-center control"
        :min="0"
        :max="100"
        hide-details
        thumb-label
        >
        <template v-slot:append>
            <v-text-field
            v-model="humidity"
            class="mt-0 pt-0"
            hide-details
            type="number"
            style="width: 60px"
            suffix="%"
            ></v-text-field>
        </template>
        </v-slider>
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
export default {
  name: "HumidityControl",

  data: () => ({
    humidity: 50,
    humidityLog: [
      { label: "10.4.", value: 0 },
      { label: "11.4.", value: 30 },
      { label: "12.4.", value: 100 },
      { label: "13.4.", value: 50 },
      { label: "14.4.", value: 70 },
      { label: "15.4.", value: 20 },
      { label: "16.4.", value: 10 },
      { label: "17.4.", value: 50 }
    ]
  }),
  computed: {
    humidityLogLabels: function() {
      return this.humidityLog.map(data => data.label);
    }
  },
  methods: {
    updateHumidityLog: function() {
      // TODO update humidity log
    }
  }
};
</script>
